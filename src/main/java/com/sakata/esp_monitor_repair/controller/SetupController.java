package com.sakata.esp_monitor_repair.controller;

import com.sakata.esp_monitor_repair.model.Machine;
import com.sakata.esp_monitor_repair.repository.MachineRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.Arrays;
import java.util.List;

@RestController
@RequestMapping("/api/setup")
public class SetupController {
    
    @Autowired
    private MachineRepository machineRepository;
    
    @PostMapping("/init-machines")
    public ResponseEntity<String> initMachines() {
        // Xóa dữ liệu cũ nếu có
        machineRepository.deleteAll();
        
        // Tạo dữ liệu mẫu
        List<Machine> machines = Arrays.asList(
            createMachine("M001", "CNC Lathe", "Line A - Station 1", "ESP32_001"),
            createMachine("M002", "Injection Molding", "Line B - Station 2", "ESP32_002"),
            createMachine("M003", "Welding Robot", "Line C - Station 3", "ESP32_003")
        );
        
        machineRepository.saveAll(machines);
        
        return ResponseEntity.ok("Đã khởi tạo " + machines.size() + " máy thành công!");
    }
    
    private Machine createMachine(String code, String name, String location, String esp32Id) {
        Machine machine = new Machine();
        machine.setMachineCode(code);
        machine.setMachineName(name);
        machine.setLocation(location);
        machine.setEsp32DeviceId(esp32Id);
        machine.setActive(true);
        return machine;
    }
}
