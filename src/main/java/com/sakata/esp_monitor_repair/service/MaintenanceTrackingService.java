package com.sakata.esp_monitor_repair.service;

import com.sakata.esp_monitor_repair.model.*;
import com.sakata.esp_monitor_repair.repository.MaintenanceRepository;
import com.sakata.esp_monitor_repair.repository.MaintenanceTrackingRepository;
import com.sakata.esp_monitor_repair.repository.TicketRepository;
import com.sakata.esp_monitor_repair.repository.MachineRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;

@Service
public class MaintenanceTrackingService {

    @Autowired
    private MaintenanceRepository maintenanceRepository;

    @Autowired
    private MachineRepository machineRepository;

    @Autowired
    private MaintenanceTrackingRepository maintenanceTrackingRepository;

    @Autowired
    private TicketRepository ticketRepository;

    // Lấy danh sách tất cả yêu cầu
    public List<MaintenanceTracking> getAllRequests() {
        return maintenanceTrackingRepository.findTop20ByOrderByTimestamp1Desc();
    }

    // Lấy yêu cầu theo trạng thái
    public List<MaintenanceTracking> getRequestsByStatus(String status) {
        return maintenanceTrackingRepository.findByStatusOrderByTimestamp1Desc(status);
    }

    // Lấy yêu cầu đang chờ cho ESP32
    @Transactional
    public Optional<MaintenanceTracking> getCurrentRequest(String espDeviceId) {
        Optional<MaintenanceTracking> resultMaintenanceTracking = null;
        Optional<MaintenanceTracking> currentMaintenanceTracking = maintenanceTrackingRepository
                .findFirstByTicket_DeviceIdAndStatusInOrderByIdAsc(
                        espDeviceId,
                        List.of("REQUESTED", "ACKNOWLEDGED", "ARRIVED"));

        if (currentMaintenanceTracking.isEmpty()) {
            System.out.println("espDeviceId:  Trống" + espDeviceId);
            Optional<MaintenanceTracking> holdingMaintenanceTracking = maintenanceTrackingRepository
                    .findFirstByTicket_DeviceIdAndStatusInOrderByIdAsc(
                            espDeviceId,
                            List.of("NONE"));

            if (holdingMaintenanceTracking.isEmpty()) {

            } else {
                holdingMaintenanceTracking.get().setStatus("REQUESTED");
                holdingMaintenanceTracking.get().setTimestamp1(LocalDateTime.now());
                maintenanceTrackingRepository.save(holdingMaintenanceTracking.get());

                Ticket ticket = holdingMaintenanceTracking.get().getTicket();
                ticket.setStatus("HANDLING");
                ticketRepository.save(ticket);

                resultMaintenanceTracking = holdingMaintenanceTracking;
            }
        } else {
            resultMaintenanceTracking = currentMaintenanceTracking;
        }

        return resultMaintenanceTracking;
    }

    // Engineer nhận yêu cầu từ ESP32 (timestamp2)
    @Transactional
    public MaintenanceTracking acknowledgeRequest(String espDeviceId, String engineerName, String requestId) {
        Optional<MaintenanceTracking> requestOpt = maintenanceTrackingRepository
                .findFirstByTicket_DeviceIdAndStatusInAndIdOrderByIdAsc(
                        espDeviceId,
                        List.of("REQUESTED"), Long.valueOf(requestId));

        if (requestOpt.isPresent()) {
            MaintenanceTracking request = requestOpt.get();
            request.setTimestamp2(LocalDateTime.now());
            request.setStatus("ACKNOWLEDGED");
            return maintenanceTrackingRepository.save(request);
        }
        throw new RuntimeException("No pending request found for device: " + espDeviceId);
    }

    // Engineer đã đến hiện trường (timestamp3)
    @Transactional
    public MaintenanceTracking arriveAtLocation(String espDeviceId, String engineerName, String requestId) {
        Optional<MaintenanceTracking> requestOpt = maintenanceTrackingRepository
                .findFirstByTicket_DeviceIdAndStatusInAndIdOrderByIdAsc(
                        espDeviceId,
                        List.of("ACKNOWLEDGED"), Long.valueOf(requestId));

        if (requestOpt.isPresent()) {
            MaintenanceTracking request = requestOpt.get();
            request.setTimestamp3(LocalDateTime.now());
            request.setStatus("ARRIVED");
            return maintenanceTrackingRepository.save(request);
        }
        throw new RuntimeException("No acknowledged request found for device: " + espDeviceId);
    }

    // Hoàn thành sửa chữa (timestamp4)
    @Transactional
    public MaintenanceTracking completeRequest(String espDeviceId, String engineerName, String requestId) {
        Optional<MaintenanceTracking> requestOpt = maintenanceTrackingRepository
                .findFirstByTicket_DeviceIdAndStatusInAndIdOrderByIdAsc(
                        espDeviceId,
                        List.of("ARRIVED"), Long.valueOf(requestId));

        if (requestOpt.isPresent()) {
            MaintenanceTracking request = requestOpt.get();
            request.setTimestamp4(LocalDateTime.now());
            request.setStatus("COMPLETED");
            return maintenanceTrackingRepository.save(request);
        }
        throw new RuntimeException("No arrived request found for device: " + espDeviceId);
    }
    // ==========================
}