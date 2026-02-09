package com.sakata.esp_monitor_repair.controller;
import com.sakata.esp_monitor_repair.model.*;
import com.sakata.esp_monitor_repair.service.MaintenanceService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@RestController
@RequestMapping("/api/maintenance")
@CrossOrigin(origins = "*")
public class MaintenanceController {
    
    @Autowired
    private MaintenanceService maintenanceService;
    
    // TQC gọi API này khi push button (timestamp1)
    @PostMapping("/request")
    public ResponseEntity<Map<String, Object>> createMaintenanceRequest(
            @RequestBody Map<String, Object> payload) {
        
        try {
            Machine machine = new Machine();
            machine.setMachineCode(payload.get("machineCode").toString());
            machine.setMachineName(payload.get("machineName").toString());
            machine.setLocation(payload.get("location").toString());
            machine.setEsp32DeviceId(payload.get("esp32DeviceId").toString());
            
            String issueDescription = payload.get("issueDescription").toString();
            
            MaintenanceRequest request = maintenanceService.createRequest(machine, issueDescription);
            
            Map<String, Object> response = new HashMap<>();
            response.put("success", true);
            response.put("requestId", request.getId());
            response.put("machineId", request.getMachine().getId());
            response.put("timestamp1", request.getTimestamp1());
            response.put("message", "Yêu cầu đã được gửi đến ESP32");
            
            return ResponseEntity.ok(response);
            
        } catch (Exception e) {
            Map<String, Object> response = new HashMap<>();
            response.put("success", false);
            response.put("message", "Lỗi: " + e.getMessage());
            return ResponseEntity.status(500).body(response);
        }
    }
    
    // ESP32 gọi API này để kiểm tra có yêu cầu mới không
    @GetMapping("/check/{esp32DeviceId}")
    public ResponseEntity<Map<String, Object>> checkPendingRequest(
            @PathVariable String esp32DeviceId) {
        
        Optional<MaintenanceRequest> request = 
            maintenanceService.getPendingRequestForDevice(esp32DeviceId);
        
        Map<String, Object> response = new HashMap<>();
        if (request.isPresent()) {
            MaintenanceRequest req = request.get();
            response.put("hasRequest", true);
            response.put("requestId", req.getId());
            response.put("machineCode", req.getMachine().getMachineCode());
            response.put("machineName", req.getMachine().getMachineName());
            response.put("location", req.getMachine().getLocation());
            response.put("issueDescription", req.getIssueDescription());
            response.put("timestamp1", req.getTimestamp1());
        } else {
            response.put("hasRequest", false);
        }
        
        return ResponseEntity.ok(response);
    }
    
    // ESP32 gọi khi engineer click button lần 1 (timestamp2 - acknowledged)
    @PostMapping("/acknowledge")
    public ResponseEntity<Map<String, Object>> acknowledgeRequest(
            @RequestBody Map<String, String> payload) {
        
        String esp32DeviceId = payload.get("esp32DeviceId");
        String engineerName = payload.getOrDefault("engineerName", "Unknown");
        
        try {
            MaintenanceRequest request = 
                maintenanceService.acknowledgeRequest(esp32DeviceId, engineerName);
            
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
        
        String esp32DeviceId = payload.get("esp32DeviceId");
        
        try {
            MaintenanceRequest request = 
                maintenanceService.arriveAtLocation(esp32DeviceId);
            
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
        
        String esp32DeviceId = payload.get("esp32DeviceId");
        
        try {
            MaintenanceRequest request = 
                maintenanceService.completeRequest(esp32DeviceId);
            
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
    public ResponseEntity<List<MaintenanceRequest>> getAllRequests() {
        return ResponseEntity.ok(maintenanceService.getAllRequests());
    }
    
    // Lấy yêu cầu theo trạng thái
    @GetMapping("/requests/status/{status}")
    public ResponseEntity<List<MaintenanceRequest>> getRequestsByStatus(
            @PathVariable MaintenanceStatus status) {
        return ResponseEntity.ok(maintenanceService.getRequestsByStatus(status));
    }
    
    // Lấy danh sách máy
    @GetMapping("/machines")
    public ResponseEntity<List<Machine>> getAllMachines() {
        return ResponseEntity.ok(maintenanceService.getAllMachines());
    }
    
    // Lấy máy theo ESP32 ID
    @GetMapping("/machines/esp32/{esp32DeviceId}")
    public ResponseEntity<Machine> getMachineByEsp32Id(@PathVariable String esp32DeviceId) {
        Optional<Machine> machine = maintenanceService.getMachineByEsp32Id(esp32DeviceId);
        return machine.map(ResponseEntity::ok)
                     .orElseGet(() -> ResponseEntity.notFound().build());
    }
}