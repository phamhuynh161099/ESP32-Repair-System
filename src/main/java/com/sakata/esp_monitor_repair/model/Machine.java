package com.sakata.esp_monitor_repair.model;

import jakarta.persistence.*;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonIgnore;
import java.util.List;

@Data
@Entity
@Table(name = "machines")
public class Machine {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    @Column(unique = true, nullable = false)
    private String machineCode;
    
    private String machineName;
    private String location;
    
    @Column(unique = true)
    private String esp32DeviceId;
    
    private boolean active;
    
    @OneToMany(mappedBy = "machine", cascade = CascadeType.ALL)
    @JsonIgnore
    private List<MaintenanceRequest> maintenanceRequests;
    
    public Machine() {
        this.active = true;
    }
}