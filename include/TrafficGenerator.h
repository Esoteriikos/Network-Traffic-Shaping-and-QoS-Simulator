#ifndef TRAFFIC_GENERATOR_H
#define TRAFFIC_GENERATOR_H

#include "Flow.h"
#include "PacketQueue.h"
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <atomic>

class TrafficGenerator {
public:
    TrafficGenerator(std::shared_ptr<PacketQueue> queue)
        : queue_(queue)
        , running_(false) {}

    ~TrafficGenerator() {
        stop();
    }

    // Add a flow to generate traffic
    void addFlow(std::shared_ptr<Flow> flow) {
        flows_.push_back(flow);
    }

    // Start generating traffic for all flows
    void start() {
        if (running_) return;
        
        running_ = true;
        
        for (auto& flow : flows_) {
            threads_.emplace_back(&TrafficGenerator::generateTraffic, this, flow);
        }
    }

    // Stop all traffic generation
    void stop() {
        if (!running_) return;
        
        running_ = false;
        
        for (auto& flow : flows_) {
            flow->setActive(false);
        }
        
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        threads_.clear();
    }

    const std::vector<std::shared_ptr<Flow>>& getFlows() const {
        return flows_;
    }

private:
    void generateTraffic(std::shared_ptr<Flow> flow) {
        while (running_ && flow->isActive()) {
            // Generate a packet
            auto packet = std::make_shared<Packet>(flow->generatePacket());
            
            // Try to enqueue the packet
            if (!queue_->enqueue(packet)) {
                // Packet was dropped due to queue overflow
                flow->recordDrop();
            }
            
            // Wait for next packet based on flow type
            uint64_t interArrival = flow->getInterArrivalTime();
            std::this_thread::sleep_for(std::chrono::microseconds(interArrival));
        }
    }

    std::shared_ptr<PacketQueue> queue_;
    std::vector<std::shared_ptr<Flow>> flows_;
    std::vector<std::thread> threads_;
    std::atomic<bool> running_;
};

#endif // TRAFFIC_GENERATOR_H
