package com.sakata.esp_monitor_repair.repository;

import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import com.sakata.esp_monitor_repair.model.LineMapEsp;

@Repository
public interface LineMapEspRepository extends JpaRepository<LineMapEsp, Long> {

    // Trả về toàn bộ object LineMapEsp dựa vào MAC address
    @Query("SELECT l FROM LineMapEsp l WHERE l.esp_mac = :macAddress")
    Optional<LineMapEsp> findByEspMac(@Param("macAddress") String macAddress);

    // Nếu bạn chỉ muốn lấy mỗi chuỗi line_id cho nhẹ thì dùng hàm này
    @Query("SELECT l.line_id FROM LineMapEsp l WHERE l.esp_mac = :macAddress")
    Optional<String> findLineIdByEspMac(@Param("macAddress") String macAddress);
}