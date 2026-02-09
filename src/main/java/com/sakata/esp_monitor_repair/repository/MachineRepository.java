package com.sakata.esp_monitor_repair.repository;

import com.sakata.esp_monitor_repair.model.Machine;
import org.springframework.data.jpa.repository.JpaRepository;
import java.util.Optional;

public interface MachineRepository extends JpaRepository<Machine, Long> {
    Optional<Machine> findByMachineCode(String machineCode);
    Optional<Machine> findByEsp32DeviceId(String esp32DeviceId);
}
