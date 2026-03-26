package com.sakata.esp_monitor_repair.model;

import jakarta.persistence.*;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonIgnore;
import java.util.List;

@Data
@Entity
@Table(name = "tbl_ticket")
public class Ticket {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private String machineCode;
    private String machineName;
    private String location;
    private String deviceId;
    private String issueDescription;

    /**
     * INIT la khoi tao, nhung ki su van chua duoc gan cho ki su de lam
     * HANDLING la dang duoc xu li
     * DONE la da duoc xu li
     */
    private String status;

    // Quan hệ 1-1 với MaintenanceTracking
    // 'mappedBy' cho JPA biết rằng entity MaintenanceTracking đang quản lý khóa
    // ngoại.
    // cascade = CascadeType.ALL giúp tự động lưu/xóa MaintenanceTracking khi Ticket
    // thay đổi.
    @OneToOne(mappedBy = "ticket", cascade = CascadeType.ALL, fetch = FetchType.LAZY, orphanRemoval = true)
    @JsonIgnore
    private MaintenanceTracking maintenanceTracking;


    // Thêm hàm này để set quan hệ 2 chiều dễ dàng và an toàn hơn ở tầng Service
    public void setMaintenanceTracking(MaintenanceTracking tracking) {
        this.maintenanceTracking = tracking;
        if (tracking != null) {
            tracking.setTicket(this);
        }
    }
}