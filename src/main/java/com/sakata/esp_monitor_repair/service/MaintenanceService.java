package com.sakata.esp_monitor_repair.service;

import com.sakata.esp_monitor_repair.model.*;
import com.sakata.esp_monitor_repair.repository.MaintenanceRepository;
import com.sakata.esp_monitor_repair.repository.MachineRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;

@Service
public class MaintenanceService {
    
    @Autowired
    private MaintenanceRepository maintenanceRepository;
    
    @Autowired
    private MachineRepository machineRepository;
    
    // TQC tạo yêu cầu (timestamp1)
    @Transactional
    public MaintenanceRequest createRequest(Machine machineData, String issueDescription) {
        // Tìm hoặc tạo mới Machine
        Machine machine = machineRepository.findByMachineCode(machineData.getMachineCode())
            .orElseGet(() -> {
                // Nếu chưa tồn tại, tạo mới
                Machine newMachine = new Machine();
                newMachine.setMachineCode(machineData.getMachineCode());
                newMachine.setMachineName(machineData.getMachineName());
                newMachine.setLocation(machineData.getLocation());
                newMachine.setEsp32DeviceId(machineData.getEsp32DeviceId());
                newMachine.setActive(true);
                return machineRepository.save(newMachine);
            });
        
        // Tạo MaintenanceRequest
        MaintenanceRequest request = new MaintenanceRequest();
        request.setMachine(machine);
        request.setIssueDescription(issueDescription);
        request.setTimestamp1(LocalDateTime.now());
        request.setStatus(MaintenanceStatus.REQUESTED);
        
        return maintenanceRepository.save(request);
    }
    
    // Engineer nhận yêu cầu từ ESP32 (timestamp2)
    @Transactional
    public MaintenanceRequest acknowledgeRequest(String esp32DeviceId, String engineerName) {
        Optional<MaintenanceRequest> requestOpt = maintenanceRepository
            .findFirstByMachine_Esp32DeviceIdAndStatusIn(
                esp32DeviceId, 
                List.of(MaintenanceStatus.REQUESTED)
            );
        
        if (requestOpt.isPresent()) {
            MaintenanceRequest request = requestOpt.get();
            request.setTimestamp2(LocalDateTime.now());
            request.setStatus(MaintenanceStatus.ACKNOWLEDGED);
            request.setEngineerName(engineerName);
            return maintenanceRepository.save(request);
        }
        throw new RuntimeException("No pending request found for device: " + esp32DeviceId);
    }
    
    // Engineer đã đến hiện trường (timestamp3)
    @Transactional
    public MaintenanceRequest arriveAtLocation(String esp32DeviceId) {
        Optional<MaintenanceRequest> requestOpt = maintenanceRepository
            .findFirstByMachine_Esp32DeviceIdAndStatusIn(
                esp32DeviceId, 
                List.of(MaintenanceStatus.ACKNOWLEDGED)
            );
        
        if (requestOpt.isPresent()) {
            MaintenanceRequest request = requestOpt.get();
            request.setTimestamp3(LocalDateTime.now());
            request.setStatus(MaintenanceStatus.ARRIVED);
            return maintenanceRepository.save(request);
        }
        throw new RuntimeException("No acknowledged request found for device: " + esp32DeviceId);
    }
    
    // Hoàn thành sửa chữa (timestamp4)
    @Transactional
    public MaintenanceRequest completeRequest(String esp32DeviceId) {
        Optional<MaintenanceRequest> requestOpt = maintenanceRepository
            .findFirstByMachine_Esp32DeviceIdAndStatusIn(
                esp32DeviceId, 
                List.of(MaintenanceStatus.ARRIVED)
            );
        
        if (requestOpt.isPresent()) {
            MaintenanceRequest request = requestOpt.get();
            request.setTimestamp4(LocalDateTime.now());
            request.setStatus(MaintenanceStatus.COMPLETED);
            return maintenanceRepository.save(request);
        }
        throw new RuntimeException("No arrived request found for device: " + esp32DeviceId);
    }
    
    // Lấy yêu cầu đang chờ cho ESP32
    public Optional<MaintenanceRequest> getPendingRequestForDevice(String esp32DeviceId) {
        return maintenanceRepository.findFirstByMachine_Esp32DeviceIdAndStatusIn(
            esp32DeviceId, 
            List.of(MaintenanceStatus.REQUESTED)
        );
    }
    
    // Lấy danh sách tất cả yêu cầu
    public List<MaintenanceRequest> getAllRequests() {
        return maintenanceRepository.findTop20ByOrderByTimestamp1Desc();
    }
    
    // Lấy yêu cầu theo trạng thái
    public List<MaintenanceRequest> getRequestsByStatus(MaintenanceStatus status) {
        return maintenanceRepository.findByStatusOrderByTimestamp1Desc(status);
    }
    
    // Lấy tất cả máy
    public List<Machine> getAllMachines() {
        return machineRepository.findAll();
    }
    
    // Lấy máy theo ESP32 ID
    public Optional<Machine> getMachineByEsp32Id(String esp32DeviceId) {
        return machineRepository.findByEsp32DeviceId(esp32DeviceId);
    }
}