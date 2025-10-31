#ifndef TRAFFIC_SHAPER_H
#define TRAFFIC_SHAPER_H

#include "TokenBucket.h"
#include "PacketQueue.h"
#include "Packet.h"
#include "Flow.h"
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>

class TrafficShaper {
public:
    TrafficShaper(std::shared_ptr<PacketQueue> inputQueue,
                  std::shared_ptr<TokenBucket> tokenBucket,
                  uint64_t linkCapacity)  // bits per second
        : inputQueue_(inputQueue)
        , tokenBucket_(tokenBucket)
        , linkCapacity_(linkCapacity)
        , running_(false)
        , packetsTransmitted_(0)
        , bytesTransmitted_(0) {}

    ~TrafficShaper() {
        stop();
    }
    
    void addFlow(std::shared_ptr<Flow> flow) {
        flows_[flow->getFlowId()] = flow;
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        thread_ = std::thread(&TrafficShaper::processPackets, this);
    }

    void stop() {
        if (!running_) return;
        
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    uint64_t getPacketsTransmitted() const { return packetsTransmitted_; }
    uint64_t getBytesTransmitted() const { return bytesTransmitted_; }

private:
    void processPackets() {
        while (running_) {
            auto packet = inputQueue_->tryDequeue();
            
            if (!packet) {
                // No packet available, sleep briefly
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            }
            
            // Try to consume tokens for this packet
            while (running_ && !tokenBucket_->consume(packet->getSize())) {
                // Not enough tokens, wait a bit
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            
            if (!running_) break;
            
            // Calculate transmission time based on link capacity
            // transmissionTime = (packetSize * 8) / linkCapacity (in seconds)
            uint64_t transmissionTimeMicros = 
                (packet->getSize() * 8 * 1000000ULL) / linkCapacity_;
            
            // Simulate transmission delay
            std::this_thread::sleep_for(std::chrono::microseconds(transmissionTimeMicros));
            
            // Mark packet as transmitted
            packet->setTransmissionTime(std::chrono::high_resolution_clock::now());
            
            packetsTransmitted_++;
            bytesTransmitted_ += packet->getSize();
            
            // Record transmission in flow statistics
            auto it = flows_.find(packet->getFlowId());
            if (it != flows_.end()) {
                double delay = std::chrono::duration<double, std::milli>(
                    packet->getTransmissionTime() - packet->getCreationTime()).count();
                it->second->recordTransmission(packet->getSize(), delay);
            }
        }
    }

    std::shared_ptr<PacketQueue> inputQueue_;
    std::shared_ptr<TokenBucket> tokenBucket_;
    uint64_t linkCapacity_;  // bits per second
    std::unordered_map<uint32_t, std::shared_ptr<Flow>> flows_;
    
    std::atomic<bool> running_;
    std::atomic<uint64_t> packetsTransmitted_;
    std::atomic<uint64_t> bytesTransmitted_;
    std::thread thread_;
};

#endif // TRAFFIC_SHAPER_H
