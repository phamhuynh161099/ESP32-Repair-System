package com.sakata.esp_monitor_repair.controller;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import com.sakata.esp_monitor_repair.model.LineMapEsp;
import com.sakata.esp_monitor_repair.repository.LineMapEspRepository;
import com.sakata.esp_monitor_repair.service.LineMapEspService;
import com.sakata.esp_monitor_repair.service.MaintenanceTrackingService;

@RestController
@RequestMapping("/api/line-esp")
@CrossOrigin("*")
public class LineMapEspController {
    @Autowired
    private LineMapEspService lineMapEspService;

    /**
     * Endpoint: GET /api/line-esp/get-line-info-by?mac=00:11:22:33:44:55
     * Mục đích: ESP gửi MAC lên để server kiểm tra xem mạch đang gắn ở Line nào.
     */
    @GetMapping("/get-line-info-by")
    public ResponseEntity<?> getLineInfoByMac(@RequestParam("mac") String macAddress) {

        Optional<?> lineOpt = lineMapEspService.getLineByEspMac(macAddress);

        if (lineOpt.isPresent()) {
            // Nếu tìm thấy, trả về mã HTTP 200 (OK) và toàn bộ object dưới dạng JSON
            return ResponseEntity.ok(lineOpt.get());
        } else {
            // Nếu không tìm thấy, trả về mã HTTP 404 (Not Found) kèm thông báo lỗi
            Map<String, String> errorResponse = new HashMap<>();
            errorResponse.put("status", "error");
            errorResponse.put("message", "Chưa có cấu hình Line cho địa chỉ MAC: " + macAddress);

            return ResponseEntity.status(HttpStatus.NOT_FOUND).body(errorResponse);
        }
    }
}
