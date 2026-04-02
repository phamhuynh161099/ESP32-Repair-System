package com.sakata.esp_monitor_repair.repository;

import java.util.List;
import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;

import com.sakata.esp_monitor_repair.model.MaintenanceTracking;

public interface MaintenanceTrackingRepository extends JpaRepository<MaintenanceTracking, Long> {
        List<MaintenanceTracking> findTop20ByOrderByTimestamp1Desc();

        List<MaintenanceTracking> findByStatusOrderByTimestamp1Desc(String status);

        Optional<MaintenanceTracking> findFirstByTicket_DeviceIdAndStatusInOrderByIdAsc(
                        String espDeviceId, List<String> statuses);

        Optional<MaintenanceTracking> findFirstByTicket_DeviceIdAndStatusInAndIdOrderByIdAsc(
                        String espDeviceId, List<String> statuses, Long id);

        // =========
        Optional<MaintenanceTracking> findFirstByTicket_MachineCodeAndStatusInOrderByIdAsc(
                        String machineCode, List<String> statuses);
}
