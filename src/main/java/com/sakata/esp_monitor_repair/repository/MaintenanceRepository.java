package com.sakata.esp_monitor_repair.repository;


import org.springframework.data.jpa.repository.JpaRepository;

import com.sakata.esp_monitor_repair.model.MaintenanceRequest;
import com.sakata.esp_monitor_repair.model.MaintenanceStatus;

import java.util.List;
import java.util.Optional;

public interface MaintenanceRepository extends JpaRepository<MaintenanceRequest, Long> {
    List<MaintenanceRequest> findByStatusOrderByTimestamp1Desc(MaintenanceStatus status);
    List<MaintenanceRequest> findTop20ByOrderByTimestamp1Desc();
    Optional<MaintenanceRequest> findFirstByMachine_Esp32DeviceIdAndStatusIn(
        String esp32DeviceId, List<MaintenanceStatus> statuses);
}