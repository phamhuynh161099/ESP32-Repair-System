package com.sakata.esp_monitor_repair.service;

import java.util.Optional;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.sakata.esp_monitor_repair.model.LineMapEsp;
import com.sakata.esp_monitor_repair.repository.LineMapEspRepository;

@Service
public class LineMapEspService {
    @Autowired
    private LineMapEspRepository lineMapEspRepository;

    public Optional<?> getLineByEspMac(String incomingMac) {
        // Tìm Line bằng địa chỉ MAC
        Optional<LineMapEsp> lineOpt = lineMapEspRepository.findByEspMac(incomingMac);

        if (lineOpt.isPresent()) {
            LineMapEsp lineMap = lineOpt.get();
            System.out.println("Đã tìm thấy mạch! Thuộc Line ID: " + lineMap.getLine_id());
            System.out.println("Tên Line: " + lineMap.getLine_name());
        } else {
            System.out.println("Cảnh báo: Địa chỉ MAC " + incomingMac + " chưa được khai báo trong hệ thống!");
        }

        return lineOpt;
    }
}
