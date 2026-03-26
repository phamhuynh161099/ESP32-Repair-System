package com.sakata.esp_monitor_repair.service;

import com.sakata.esp_monitor_repair.model.*;
import com.sakata.esp_monitor_repair.repository.MaintenanceRepository;
import com.sakata.esp_monitor_repair.repository.MaintenanceTrackingRepository;
import com.sakata.esp_monitor_repair.repository.TicketRepository;
import lombok.extern.slf4j.Slf4j; // Thêm thư viện Log
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@Slf4j // Thêm annotation này để dùng log.error
@Service
public class TicketService {
    @Autowired
    private TicketRepository ticketRepository;

    @Autowired
    private MaintenanceTrackingRepository maintenanceTrackingRepository;

    // Nên mở comment @Transactional để đảm bảo dữ liệu toàn vẹn nếu save table 2 bị
    // lỗi
    @Transactional
    public Object handleDeviceTicket(Map<String, Object> payload) {
        try {
            Map<String, Object> result = new HashMap<>();
            List<String> activeStatuses = Arrays.asList("INIT", "HANDLING");

            // Cách lấy dữ liệu an toàn, tránh NullPointerException
            String deviceId = payload.getOrDefault("deviceId", "").toString();
            if (deviceId.isEmpty()) {
                log.warn("Payload không chứa deviceId!");
                return null; // Hoặc ném ra Exception tùy logic của bạn
            }

            Optional<Ticket> existingTicketOpt = ticketRepository
                    .findFirstByDeviceIdAndStatusIn(deviceId, activeStatuses);

            Ticket newTicket = new Ticket();
            newTicket.setDeviceId(deviceId);
            // Sử dụng getOrDefault để tránh NullPointerException khi lấy các field khác
            newTicket.setIssueDescription(payload.getOrDefault("issueDescription", "").toString());
            newTicket.setMachineCode(payload.getOrDefault("machineCode", "").toString());
            newTicket.setMachineName(payload.getOrDefault("machineName", "").toString());
            newTicket.setLocation(payload.getOrDefault("location", "").toString());

            MaintenanceTracking maintenanceTracking = new MaintenanceTracking();

            if (existingTicketOpt.isEmpty()) {
                newTicket.setStatus("HANDLING");
                maintenanceTracking.setStatus("REQUESTED");
                maintenanceTracking.setTimestamp1(LocalDateTime.now());
            } else {
                newTicket.setStatus("INIT");
                maintenanceTracking.setStatus("NONE");
            }

            var dataSavedTicked = ticketRepository.save(newTicket);
            maintenanceTracking.setTicket(dataSavedTicked);
            var dataSavedMaintenanceTracking = maintenanceTrackingRepository.save(maintenanceTracking);

            // Tạm thời trả về map result thay vì null để dễ track kết quả
            result.put("status", "success");
            result.put("ticked", dataSavedTicked);
            result.put("maintenanceTracking", dataSavedMaintenanceTracking);
            result.put("ticketId", dataSavedTicked.getId());
            return result;

        } catch (Exception e) {
            // BẮT BUỘC LOG LỖI RA CONSOLE
            log.error("Lỗi nghiêm trọng xảy ra trong hàm handleDeviceTicket: ", e);
            throw e; // Ném ngược lỗi ra ngoài để Transactional có thể rollback dữ liệu
        }
    }
}