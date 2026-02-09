package com.sakata.esp_monitor_repair.model;

import jakarta.persistence.*;
import lombok.Data;
import java.time.LocalDateTime;

@Data
@Entity
@Table(name = "maintenance_requests")
public class MaintenanceRequest {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    @ManyToOne
    @JoinColumn(name = "machine_id")
    private Machine machine;
    
    private String issueDescription;
    
    @Enumerated(EnumType.STRING)
    private MaintenanceStatus status;
    
    private LocalDateTime timestamp1; // TQC push button
    private LocalDateTime timestamp2; // Engineer acknowledged
    private LocalDateTime timestamp3; // Engineer arrived
    private LocalDateTime timestamp4; // Completed fix
    
    private String engineerName;
    
    public MaintenanceRequest() {
        this.status = MaintenanceStatus.REQUESTED;
        this.timestamp1 = LocalDateTime.now();
    }
    
    public long getResponseTime() {
        if (timestamp2 != null && timestamp1 != null) {
            return java.time.Duration.between(timestamp1, timestamp2).getSeconds();
        }
        return 0;
    }
    
    public long getArrivalTime() {
        if (timestamp3 != null && timestamp2 != null) {
            return java.time.Duration.between(timestamp2, timestamp3).getSeconds();
        }
        return 0;
    }
    
    public long getFixTime() {
        if (timestamp4 != null && timestamp3 != null) {
            return java.time.Duration.between(timestamp3, timestamp4).getSeconds();
        }
        return 0;
    }
    
    public long getTotalTime() {
        if (timestamp4 != null && timestamp1 != null) {
            return java.time.Duration.between(timestamp1, timestamp4).getSeconds();
        }
        return 0;
    }
}
