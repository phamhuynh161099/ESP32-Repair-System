// API Base URL
const API_BASE = '/api/maintenance';

// State management
let currentFilter = 'all';
let refreshInterval;
let lastDataJson = ''; // Biến mới: Lưu trữ dữ liệu cũ để so sánh

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    initForm();
    initTabs();
    loadRequests();
    startAutoRefresh();
});

// Form submission
function initForm() {
    const form = document.getElementById('callServiceForm');
    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        const formData = new FormData(form);
        const payload = {
            machineId: Date.now(), // Generate ID
            machineCode: formData.get('machineCode'),
            machineName: formData.get('machineName'),
            location: formData.get('location'),
            esp32DeviceId: formData.get('esp32DeviceId'),
            issueDescription: formData.get('issueDescription')
        };
        
        try {
            const response = await fetch(`${API_BASE}/request`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(payload)
            });
            
            const result = await response.json();
            
            if (result.success) {
                showToast('<i class="fa-solid fa-check-to-slot"></i>Yêu cầu đã được gửi thành công!', 'success');
                form.reset();
                lastDataJson = ''; // Ép buộc xóa cache để render lại ngay lập tức
                loadRequests();
            } else {
                showToast('❌ Có lỗi xảy ra. Vui lòng thử lại!', 'error');
            }
        } catch (error) {
            console.error('Error:', error);
            showToast('❌ Không thể kết nối đến server!', 'error');
        }
    });
}

// Tab filtering
function initTabs() {
    const tabs = document.querySelectorAll('.tab-btn');
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            tabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            currentFilter = tab.dataset.status;
            
            // Xóa HTML hiện tại và reset cache khi chuyển tab để có cảm giác phản hồi ngay lập tức
            const container = document.getElementById('requestsList');
            container.innerHTML = '<div class="loading">Đang tải dữ liệu...</div>';
            lastDataJson = ''; 
            
            loadRequests();
        });
    });
}

// Load requests (Đã tối ưu chống chớp giật)
async function loadRequests() {
    const container = document.getElementById('requestsList');
    
    // Chỉ hiện "Loading" ở lần đầu tiên tải trang hoặc khi danh sách đang trống
    if (!container.innerHTML || container.querySelector('.loading')) {
        container.innerHTML = '<div class="loading">Đang tải dữ liệu...</div>';
    }
    
    try {
        let url = `${API_BASE}/requests`;
        if (currentFilter !== 'all') {
            url = `${API_BASE}/requests/status/${currentFilter}`;
        }
        
        const response = await fetch(url);
        const requests = await response.json();
        
        // Chuyển dữ liệu mới thành chuỗi để so sánh
        const currentDataJson = JSON.stringify(requests);
        
        // Nếu dữ liệu không có gì thay đổi so với lần trước -> Thoát hàm, không vẽ lại HTML
        if (currentDataJson === lastDataJson) {
            return; 
        }
        
        // Cập nhật lại dữ liệu cũ bằng dữ liệu mới
        lastDataJson = currentDataJson;
        
        if (requests.length === 0) {
            container.innerHTML = '<div class="loading">Không có yêu cầu nào</div>';
            updateStatusCounts(requests); // Vẫn cập nhật bộ đếm
            return;
        }
        
        // Chỉ ghi đè HTML khi thực sự có dữ liệu mới
        container.innerHTML = requests.map(req => renderRequestCard(req)).join('');
        updateStatusCounts(requests);
        
    } catch (error) {
        console.error('Error loading requests:', error);
        // Nếu đang có lỗi và màn hình đang trống/loading thì mới hiện lỗi
        // Tránh tình trạng đang có danh sách mà rớt mạng 1 giây lại bị chớp màn hình lỗi
        if (container.querySelector('.loading')) {
            container.innerHTML = '<div class="loading">Lỗi khi tải dữ liệu</div>';
        }
    }
}

// Render request card
function renderRequestCard(request) {
    const statusText = {
        'REQUESTED': 'Waiting',
        'ACKNOWLEDGED': 'Acknowledge',
        'ARRIVED': 'Fixing',
        'COMPLETED': 'Completed'
    };
    
    return `
        <div class="request-card">
            <div class="request-header">
                <div class="machine-info">
                    ${request.machine.machineCode} - ${request.machine.machineName}
                </div>
                <span class="status-badge status-${request.status}">
                    ${statusText[request.status]}
                </span>
            </div>
            <div class="request-details">
                <p><strong><i class="fa-solid fa-map-pin"></i> Location:</strong> ${request.machine.location}</p>
                <p><strong><i class="fa-solid fa-gears"></i> Issue:</strong> ${request.issueDescription}</p>
                ${request.engineerName ? `<p><strong><i class="fa-solid fa-user-gear"></i> Engineer Name:</strong> ${request.engineerName}</p>` : ''}
                ${request.status === 'COMPLETED' ? `
                    <p><strong><i class="fa-solid fa-clock"></i> Total Time:</strong> ${formatTime(request.totalTime)}</p>
                ` : ''}
            </div>
            ${renderTimeline(request)}
        </div>
    `;
}

// Render timeline
function renderTimeline(request) {
    const timestamps = [];
    
    if (request.timestamp1) {
        timestamps.push(`
            <div class="timeline-item">
                <span class="icon"><i class="fa-solid fa-phone"></i></span>
                <span>Yêu cầu: ${formatDateTime(request.timestamp1)}</span>
            </div>
        `);
    }
    
    if (request.timestamp2) {
        timestamps.push(`
            <div class="timeline-item">
                <span class="icon"><i class="fa-solid fa-check-to-slot"></i></span>
                <span>Đã nhận: ${formatDateTime(request.timestamp2)} (${formatTime(request.responseTime)})</span>
            </div>
        `);
    }
    
    if (request.timestamp3) {
        timestamps.push(`
            <div class="timeline-item">
                <span class="icon"><i class="fa-solid fa-car"></i></span>
                <span>Đã đến: ${formatDateTime(request.timestamp3)} (${formatTime(request.arrivalTime)})</span>
            </div>
        `);
    }
    
    if (request.timestamp4) {
        timestamps.push(`
            <div class="timeline-item">
                <span class="icon"><i class="fa-solid fa-check-to-slot"></i></span>
                <span>Hoàn thành: ${formatDateTime(request.timestamp4)} (${formatTime(request.fixTime)})</span>
            </div>
        `);
    }
    
    return timestamps.length > 0 ? `<div class="timeline">${timestamps.join('')}</div>` : '';
}

// Update status counts
function updateStatusCounts(requests) {
    const pending = requests.filter(r => r.status === 'REQUESTED').length;
    const processing = requests.filter(r => ['ACKNOWLEDGED', 'ARRIVED'].includes(r.status)).length;
    const completed = requests.filter(r => r.status === 'COMPLETED').length;
    
    document.getElementById('pendingCount').textContent = pending;
    document.getElementById('processingCount').textContent = processing;
    document.getElementById('completedCount').textContent = completed;
}

// Auto refresh
function startAutoRefresh() {
    refreshInterval = setInterval(() => {
        loadRequests();
    }, 5000); // Refresh every 5 seconds
}

// Utility functions
function formatDateTime(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleString('vi-VN');
}

function formatTime(seconds) {
    if (!seconds && seconds !== 0) return '';
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}m ${secs}s`;
}

function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = `toast ${type} show`;
    
    setTimeout(() => {
        toast.classList.remove('show');
    }, 3000);
}