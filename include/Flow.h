#ifndef FLOW_H
#define FLOW_H

#include "Packet.h"
#include <string>
#include <atomic>
#include <random>

enum class FlowType {
    CONSTANT_RATE,    // Constant bit rate
    BURSTY,          // Bursty traffic
    POISSON          // Poisson arrival process
};

class Flow {
public:
    Flow(uint32_t flowId, FlowType type, uint64_t targetRate, 
         PacketPriority priority = PacketPriority::MEDIUM)
        : flowId_(flowId)
        , type_(type)
        , targetRate_(targetRate)  // Target rate in bytes/sec
        , priority_(priority)
        , active_(true)
        , packetsSent_(0)
        , packetsDropped_(0)
        , bytesTransmitted_(0)
        , totalDelay_(0.0)
        , generator_(std::random_device{}()) {}

    uint32_t getFlowId() const { return flowId_; }
    FlowType getType() const { return type_; }
    uint64_t getTargetRate() const { return targetRate_; }
    PacketPriority getPriority() const { return priority_; }
    bool isActive() const { return active_; }

    void setActive(bool active) { active_ = active; }
    
    // Generate next packet based on flow type
    Packet generatePacket(uint32_t minSize = 64, uint32_t maxSize = 1500) {
        packetsSent_++;
        
        std::uniform_int_distribution<uint32_t> sizeDist(minSize, maxSize);
        uint32_t packetSize = sizeDist(generator_);
        
        return Packet(flowId_, packetSize, priority_);
    }

    // Get inter-arrival time in microseconds based on flow type
    uint64_t getInterArrivalTime(uint32_t avgPacketSize = 500) {
        switch (type_) {
            case FlowType::CONSTANT_RATE: {
                // Constant inter-arrival time
                return (avgPacketSize * 1000000ULL) / targetRate_;
            }
            case FlowType::BURSTY: {
                // Alternating between burst and idle periods
                std::uniform_real_distribution<double> dist(0.0, 1.0);
                if (dist(generator_) < 0.3) { // 30% chance of burst
                    return (avgPacketSize * 1000000ULL) / (targetRate_ * 3); // 3x rate
                } else {
                    return (avgPacketSize * 1000000ULL) / (targetRate_ / 2); // 0.5x rate
                }
            }
            case FlowType::POISSON: {
                // Exponentially distributed inter-arrival times
                std::exponential_distribution<double> dist(
                    static_cast<double>(targetRate_) / avgPacketSize);
                return static_cast<uint64_t>(dist(generator_) * 1000000.0);
            }
        }
        return (avgPacketSize * 1000000ULL) / targetRate_;
    }

    // Statistics
    void recordDrop() { packetsDropped_++; }
    void recordTransmission(uint32_t bytes, double delay) {
        bytesTransmitted_ += bytes;
        double current = totalDelay_.load();
        while (!totalDelay_.compare_exchange_weak(current, current + delay));
    }

    uint64_t getPacketsSent() const { return packetsSent_; }
    uint64_t getPacketsDropped() const { return packetsDropped_; }
    uint64_t getBytesTransmitted() const { return bytesTransmitted_; }
    double getAverageDelay() const {
        uint64_t transmitted = packetsSent_ - packetsDropped_;
        return transmitted > 0 ? totalDelay_ / transmitted : 0.0;
    }

private:
    uint32_t flowId_;
    FlowType type_;
    uint64_t targetRate_;
    PacketPriority priority_;
    std::atomic<bool> active_;
    
    std::atomic<uint64_t> packetsSent_;
    std::atomic<uint64_t> packetsDropped_;
    std::atomic<uint64_t> bytesTransmitted_;
    std::atomic<double> totalDelay_;
    
    std::mt19937 generator_;
};

#endif // FLOW_H
