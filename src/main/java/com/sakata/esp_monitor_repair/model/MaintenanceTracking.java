package com.sakata.esp_monitor_repair.model;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

@Entity
@Table(name = "maintenance_trackings")
@Getter
@Setter
@NoArgsConstructor
public class MaintenanceTracking {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // @Enumerated(EnumType.STRING)
    private String status;

    private LocalDateTime timestamp1; // TQC push button
    private LocalDateTime timestamp2; // Engineer acknowledged
    private LocalDateTime timestamp3; // Completed fix
    // private LocalDateTime timestamp4; // Completed fix
    
    public long getResponseTime() {
        if (timestamp2 != null && timestamp1 != null) {
            return java.time.Duration.between(timestamp1, timestamp2).getSeconds();
        }
        return 0;
    }
    
    public long getFixTime() {
        if (timestamp3 != null && timestamp2 != null) {
            return java.time.Duration.between(timestamp2, timestamp3).getSeconds();
        }
        return 0;
    }
    
    // public long getFixTime() {
    //     if (timestamp4 != null && timestamp3 != null) {
    //         return java.time.Duration.between(timestamp3, timestamp4).getSeconds();
    //     }
    //     return 0;
    // }
    
    public long getTotalTime() {
        if (timestamp3 != null && timestamp1 != null) {
            return java.time.Duration.between(timestamp1, timestamp3).getSeconds();
        }
        return 0;
    }

    // Khóa ngoại ticket_id liên kết với bảng tickets
    @OneToOne()
    @JoinColumn(name = "ticket_id", referencedColumnName = "id", unique = true, nullable = false)
    private Ticket ticket;
}