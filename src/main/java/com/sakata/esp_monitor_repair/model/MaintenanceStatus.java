package com.sakata.esp_monitor_repair.model;

public enum MaintenanceStatus {
    NONE,           // Engineer chưa chấp nhận request này
    REQUESTED,      // TQC đã gọi
    ACKNOWLEDGED,   // Engineer đã nhận (timestamp2)
    ARRIVED,        // Engineer đã đến (timestamp3)
    COMPLETED       // Hoàn thành sửa chữa (timestamp4)
}
