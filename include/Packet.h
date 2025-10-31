#ifndef PACKET_H
#define PACKET_H

#include <chrono>
#include <cstdint>

enum class PacketPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

class Packet {
public:
    using TimePoint = std::chrono::high_resolution_clock::time_point;

    Packet(uint32_t flowId, uint32_t size, PacketPriority priority = PacketPriority::MEDIUM)
        : flowId_(flowId)
        , size_(size)
        , priority_(priority)
        , creationTime_(std::chrono::high_resolution_clock::now())
        , transmissionTime_()
        , dropped_(false) {}

    uint32_t getFlowId() const { return flowId_; }
    uint32_t getSize() const { return size_; }
    PacketPriority getPriority() const { return priority_; }
    TimePoint getCreationTime() const { return creationTime_; }
    TimePoint getTransmissionTime() const { return transmissionTime_; }
    bool isDropped() const { return dropped_; }

    void setTransmissionTime(TimePoint time) { transmissionTime_ = time; }
    void markDropped() { dropped_ = true; }

    // Calculate delay in milliseconds
    double getDelay() const {
        if (dropped_ || transmissionTime_ == TimePoint{}) {
            return -1.0;
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            transmissionTime_ - creationTime_);
        return duration.count() / 1000.0;
    }

private:
    uint32_t flowId_;
    uint32_t size_;           // Size in bytes
    PacketPriority priority_;
    TimePoint creationTime_;
    TimePoint transmissionTime_;
    bool dropped_;
};

#endif // PACKET_H
