package com.sakata.esp_monitor_repair.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.sakata.esp_monitor_repair.model.MaintenanceRequest;
import com.sakata.esp_monitor_repair.model.MaintenanceStatus;
import com.sakata.esp_monitor_repair.model.MaintenanceTracking;

public interface MaintenanceTrackingRepository extends JpaRepository<MaintenanceTracking, Long> {
    List<MaintenanceTracking> findTop20ByOrderByTimestamp1Desc();
    List<MaintenanceTracking> findByStatusOrderByTimestamp1Desc(String status);
}
