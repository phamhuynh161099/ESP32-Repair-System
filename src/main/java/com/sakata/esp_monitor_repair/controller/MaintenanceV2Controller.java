package com.sakata.esp_monitor_repair.controller;

import com.sakata.esp_monitor_repair.model.*;
import com.sakata.esp_monitor_repair.service.MaintenanceService;
import com.sakata.esp_monitor_repair.service.MaintenanceTrackingService;
import com.sakata.esp_monitor_repair.service.TicketService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@RestController
@RequestMapping("/api/maintenance-v2")
@CrossOrigin(origins = "*")
public class MaintenanceV2Controller {

    @Autowired
    private MaintenanceService maintenanceService;

    @Autowired
    private TicketService ticketService;

    @Autowired
    private MaintenanceTrackingService maintenanceTrackingService;

    /**
     * Timestamp 1, Khi nguoi dung nhan nut goi engineer
     * Cac ma code thiet bi ESP_001 ESP_002
     */
    @PostMapping("/request")
    public ResponseEntity<Map<String, Object>> createMaintenanceRequest(
            @RequestBody Map<String, Object> payload) {

        try {
            // Khoi tao ticked
            var initTicket = ticketService.handleDeviceTicket(payload);

            Map<String, Object> response = new HashMap<>();
            response.put("success", true);
            response.put("data", initTicket);
            response.put("message", "Yêu cầu đã được gửi đến ESP32");

            return ResponseEntity.ok(response);

        } catch (Exception e) {
            Map<String, Object> response = new HashMap<>();
            response.put("success", false);
            response.put("message", "Lỗi: " + e.getMessage());
            return ResponseEntity.status(500).body(response);
        }
    }

    // ESP32 gọi khi engineer click button lần 1 (timestamp2 - acknowledged)
    @PostMapping("/acknowledge")
    public ResponseEntity<Map<String, Object>> acknowledgeRequest(
            @RequestBody Map<String, String> payload) {

        String espDeviceId = payload.get("esp32DeviceId");
        String engineerName = payload.getOrDefault("engineerName", "Unknown");

        String requestId = payload.getOrDefault("requestId", "Unknown");
        System.out.println("::acknowledge:  " + espDeviceId);
        try {
            MaintenanceTracking request = maintenanceTrackingService.acknowledgeRequest(espDeviceId, engineerName,
                    requestId);

            Map<String, Object> response = new HashMap<>();
            response.put("success", true);
            response.put("requestId", request.getId());
            response.put("timestamp2", request.getTimestamp2());
            response.put("status", request.getStatus());
            response.put("message", "Engineer đã nhận yêu cầu");

            return ResponseEntity.ok(response);
        } catch (RuntimeException e) {
            Map<String, Object> response = new HashMap<>();
            response.put("success", false);
            response.put("message", e.getMessage());
            return ResponseEntity.badRequest().body(response);
        }
    }

    // ESP32 gọi khi engineer đến nơi (timestamp3 - arrived)
    @PostMapping("/arrive")
    public ResponseEntity<Map<String, Object>> arriveAtLocation(
            @RequestBody Map<String, String> payload) {

        String espDeviceId = payload.get("esp32DeviceId");
        String engineerName = payload.getOrDefault("engineerName", "Unknown");
        String requestId = payload.getOrDefault("requestId", null);

        try {
            MaintenanceTracking request = maintenanceTrackingService.arriveAtLocation(espDeviceId, engineerName,
                    requestId);

            Map<String, Object> response = new HashMap<>();
            response.put("success", true);
            response.put("requestId", request.getId());
            response.put("timestamp3", request.getTimestamp3());
            response.put("status", request.getStatus());
            response.put("message", "Engineer đã đến hiện trường");

            return ResponseEntity.ok(response);
        } catch (RuntimeException e) {
            Map<String, Object> response = new HashMap<>();
            response.put("success", false);
            response.put("message", e.getMessage());
            return ResponseEntity.badRequest().body(response);
        }
    }

    // ESP32 gọi khi hoàn thành (timestamp4 - completed)
    @PostMapping("/complete")
    public ResponseEntity<Map<String, Object>> completeRequest(
            @RequestBody Map<String, String> payload) {

        String espDeviceId = payload.get("esp32DeviceId");
        String engineerName = payload.getOrDefault("engineerName", "Unknown");
        String requestId = payload.getOrDefault("requestId", null);

        try {
            MaintenanceTracking request = maintenanceTrackingService.completeRequest(espDeviceId, engineerName,
                    requestId);

            Map<String, Object> response = new HashMap<>();
            response.put("success", true);
            response.put("requestId", request.getId());
            response.put("timestamp4", request.getTimestamp4());
            response.put("status", request.getStatus());
            response.put("totalTime", request.getTotalTime());
            response.put("message", "Đã hoàn thành sửa chữa");

            return ResponseEntity.ok(response);
        } catch (RuntimeException e) {
            Map<String, Object> response = new HashMap<>();
            response.put("success", false);
            response.put("message", e.getMessage());
            return ResponseEntity.badRequest().body(response);
        }
    }

    // Lấy danh sách tất cả yêu cầu
    @GetMapping("/requests")
    public ResponseEntity<List<MaintenanceTracking>> getAllRequests() {
        return ResponseEntity.ok(maintenanceTrackingService.getAllRequests());
    }

    // Lấy yêu cầu theo trạng thái
    @GetMapping("/requests/status/{status}")
    public ResponseEntity<List<MaintenanceTracking>> getRequestsByStatus(
            @PathVariable String status) {
        return ResponseEntity.ok(maintenanceTrackingService.getRequestsByStatus(status));
    }

    // ESP32 gọi API này để kiểm tra có yêu cầu mới không !!!!!!!
    @GetMapping("/check/{espDeviceId}")
    public ResponseEntity<Map<String, Object>> checkPendingRequest(
            @PathVariable String espDeviceId) {

        Optional<MaintenanceTracking> request = maintenanceTrackingService.getCurrentRequest(espDeviceId);

        Map<String, Object> response = new HashMap<>();
        if (request.isPresent()) {
            MaintenanceTracking req = request.get();
            response.put("hasRequest", true);
            response.put("requestId", req.getId());
            response.put("status", req.getStatus());
            response.put("machineCode", req.getTicket().getMachineCode());
            response.put("machineName", req.getTicket().getMachineName());
            response.put("location", req.getTicket().getLocation());
            response.put("issueDescription", req.getTicket().getIssueDescription());
            response.put("timestamp1", req.getTimestamp1());
        } else {
            response.put("hasRequest", false);
        }

        return ResponseEntity.ok(response);
    }
    // ======================================
}