#ifndef STATISTICS_COLLECTOR_H
#define STATISTICS_COLLECTOR_H

#include "Flow.h"
#include "PacketQueue.h"
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <iomanip>
#include <iostream>

struct FlowStats {
    uint32_t flowId;
    uint64_t packetsSent;
    uint64_t packetsDropped;
    uint64_t bytesTransmitted;
    double averageDelay;
    double throughput;  // bytes per second
    double dropRate;
};

struct SystemStats {
    double timestamp;  // seconds from start
    size_t queueOccupancy;
    uint64_t totalPacketsTransmitted;
    uint64_t totalBytesTransmitted;
    double aggregateThroughput;
    std::vector<FlowStats> flowStats;
};

class StatisticsCollector {
public:
    StatisticsCollector(const std::vector<std::shared_ptr<Flow>>& flows,
                       std::shared_ptr<PacketQueue> queue)
        : flows_(flows)
        , queue_(queue)
        , running_(false)
        , startTime_(std::chrono::high_resolution_clock::now())
        , sampleInterval_(100)  // Sample every 100ms
        , lastByteCount_(0)
        , lastPacketCount_(0) {}

    ~StatisticsCollector() {
        stop();
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        startTime_ = std::chrono::high_resolution_clock::now();
        thread_ = std::thread(&StatisticsCollector::collectStats, this);
    }

    void stop() {
        if (!running_) return;
        
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void setSampleInterval(uint32_t intervalMs) {
        sampleInterval_ = intervalMs;
    }

    const std::vector<SystemStats>& getHistory() const {
        return history_;
    }

    void saveToCSV(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return;
        }

        // Write header
        file << "Timestamp,QueueOccupancy,TotalPackets,TotalBytes,AggregateThroughput";
        for (const auto& flow : flows_) {
            file << ",Flow" << flow->getFlowId() << "_Throughput"
                 << ",Flow" << flow->getFlowId() << "_Delay"
                 << ",Flow" << flow->getFlowId() << "_DropRate";
        }
        file << "\n";

        // Write data
        for (const auto& stats : history_) {
            file << std::fixed << std::setprecision(3) << stats.timestamp << ","
                 << stats.queueOccupancy << ","
                 << stats.totalPacketsTransmitted << ","
                 << stats.totalBytesTransmitted << ","
                 << stats.aggregateThroughput;
            
            for (const auto& flowStat : stats.flowStats) {
                file << "," << flowStat.throughput
                     << "," << flowStat.averageDelay
                     << "," << flowStat.dropRate;
            }
            file << "\n";
        }

        file.close();
    }

    void printSummary() const {
        if (history_.empty()) {
            std::cout << "No statistics collected.\n";
            return;
        }

        const auto& lastStats = history_.back();
        
        std::cout << "\n========== Simulation Summary ==========\n";
        std::cout << "Duration: " << lastStats.timestamp << " seconds\n";
        std::cout << "Total Packets Transmitted: " << lastStats.totalPacketsTransmitted << "\n";
        std::cout << "Total Bytes Transmitted: " << lastStats.totalBytesTransmitted << "\n";
        std::cout << "Average Aggregate Throughput: " 
                  << (lastStats.aggregateThroughput / 1024.0) << " KB/s\n\n";

        std::cout << "Per-Flow Statistics:\n";
        std::cout << std::setw(8) << "FlowID" 
                  << std::setw(12) << "Sent"
                  << std::setw(12) << "Dropped"
                  << std::setw(12) << "DropRate%"
                  << std::setw(15) << "Throughput(KB/s)"
                  << std::setw(15) << "AvgDelay(ms)\n";
        std::cout << std::string(74, '-') << "\n";

        for (const auto& flowStat : lastStats.flowStats) {
            std::cout << std::setw(8) << flowStat.flowId
                      << std::setw(12) << flowStat.packetsSent
                      << std::setw(12) << flowStat.packetsDropped
                      << std::setw(12) << std::fixed << std::setprecision(2) 
                      << (flowStat.dropRate * 100.0)
                      << std::setw(15) << std::fixed << std::setprecision(2)
                      << (flowStat.throughput / 1024.0)
                      << std::setw(15) << std::fixed << std::setprecision(3)
                      << flowStat.averageDelay << "\n";
        }
        std::cout << "========================================\n\n";
    }

private:
    void collectStats() {
        while (running_) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - startTime_).count();

            SystemStats stats;
            stats.timestamp = elapsed;
            stats.queueOccupancy = queue_->size();
            
            // Collect per-flow statistics
            uint64_t totalBytes = 0;
            uint64_t totalPackets = 0;
            
            for (const auto& flow : flows_) {
                FlowStats flowStat;
                flowStat.flowId = flow->getFlowId();
                flowStat.packetsSent = flow->getPacketsSent();
                flowStat.packetsDropped = flow->getPacketsDropped();
                flowStat.bytesTransmitted = flow->getBytesTransmitted();
                flowStat.averageDelay = flow->getAverageDelay();
                
                // Calculate throughput over sample interval
                flowStat.throughput = elapsed > 0 ? 
                    flowStat.bytesTransmitted / elapsed : 0.0;
                
                // Calculate drop rate
                flowStat.dropRate = flowStat.packetsSent > 0 ?
                    static_cast<double>(flowStat.packetsDropped) / flowStat.packetsSent : 0.0;
                
                stats.flowStats.push_back(flowStat);
                
                totalBytes += flowStat.bytesTransmitted;
                totalPackets += (flowStat.packetsSent - flowStat.packetsDropped);
            }
            
            stats.totalBytesTransmitted = totalBytes;
            stats.totalPacketsTransmitted = totalPackets;
            stats.aggregateThroughput = elapsed > 0 ? totalBytes / elapsed : 0.0;
            
            history_.push_back(stats);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(sampleInterval_));
        }
    }

    std::vector<std::shared_ptr<Flow>> flows_;
    std::shared_ptr<PacketQueue> queue_;
    std::atomic<bool> running_;
    std::chrono::high_resolution_clock::time_point startTime_;
    uint32_t sampleInterval_;
    std::vector<SystemStats> history_;
    std::thread thread_;
    
    uint64_t lastByteCount_;
    uint64_t lastPacketCount_;
};

#endif // STATISTICS_COLLECTOR_H
