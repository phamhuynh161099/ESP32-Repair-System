package com.sakata.esp_monitor_repair.repository;

import com.sakata.esp_monitor_repair.model.Machine;
import com.sakata.esp_monitor_repair.model.Ticket;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface TicketRepository extends JpaRepository<Ticket, Long> {
    // Query: Tìm xem có ticket nào của deviceId này đang ở các trạng thái được
    // truyền vào hay không
    boolean existsByDeviceIdAndStatusIn(String deviceId, List<String> statuses);

    // Bổ sung thêm hàm này nếu bạn muốn lấy hẳn record đó ra để xem hoặc update
    Optional<Ticket> findFirstByDeviceIdAndStatusIn(String deviceId, List<String> statuses);
}
