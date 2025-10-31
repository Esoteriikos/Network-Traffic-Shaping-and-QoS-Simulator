#include "Flow.h"
#include "Packet.h"
#include "PacketQueue.h"
#include "TokenBucket.h"
#include "TrafficGenerator.h"
#include "TrafficShaper.h"
#include "StatisticsCollector.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

void printBanner() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   Network Traffic Shaping and QoS Simulator                  ║\n";
    std::cout << "║   Token Bucket Filter (TBF) Implementation                   ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void printConfiguration(uint64_t linkCapacity, uint64_t tokenRate, 
                       uint64_t bucketSize, size_t queueSize) {
    std::cout << "Simulation Configuration:\n";
    std::cout << "-------------------------\n";
    std::cout << "Link Capacity:     " << (linkCapacity / 1000000) << " Mbps\n";
    std::cout << "Token Rate:        " << (tokenRate / 1024) << " KB/s\n";
    std::cout << "Bucket Size:       " << (bucketSize / 1024) << " KB\n";
    std::cout << "Max Queue Size:    " << queueSize << " packets\n";
    std::cout << "\n";
}

void runScenario1() {
    std::cout << "\n========== Scenario 1: Basic Traffic Shaping ==========\n";
    std::cout << "Testing TBF with 3 constant-rate flows\n";
    std::cout << "Observing queue behavior and flow fairness\n\n";

    // Network parameters
    uint64_t linkCapacity = 10 * 1000000;  // 10 Mbps
    uint64_t tokenRate = 800 * 1024;       // 800 KB/s
    uint64_t bucketSize = 100 * 1024;      // 100 KB
    size_t queueSize = 500;                // 500 packets

    printConfiguration(linkCapacity, tokenRate, bucketSize, queueSize);

    // Create components
    auto queue = std::make_shared<PacketQueue>(queueSize);
    auto tokenBucket = std::make_shared<TokenBucket>(tokenRate, bucketSize);
    
    // Create flows
    auto flow1 = std::make_shared<Flow>(1, FlowType::CONSTANT_RATE, 
                                        400 * 1024, PacketPriority::MEDIUM);
    auto flow2 = std::make_shared<Flow>(2, FlowType::CONSTANT_RATE, 
                                        400 * 1024, PacketPriority::MEDIUM);
    auto flow3 = std::make_shared<Flow>(3, FlowType::CONSTANT_RATE, 
                                        400 * 1024, PacketPriority::MEDIUM);
    
    std::vector<std::shared_ptr<Flow>> flows = {flow1, flow2, flow3};
    
    std::cout << "Flows:\n";
    for (const auto& flow : flows) {
        std::cout << "  Flow " << flow->getFlowId() 
                  << ": " << (flow->getTargetRate() / 1024) << " KB/s (CONSTANT_RATE)\n";
    }
    std::cout << "\n";

    // Create traffic generator and shaper
    auto generator = std::make_shared<TrafficGenerator>(queue);
    for (const auto& flow : flows) {
        generator->addFlow(flow);
    }
    
    auto shaper = std::make_shared<TrafficShaper>(queue, tokenBucket, linkCapacity);
    for (const auto& flow : flows) {
        shaper->addFlow(flow);
    }
    
    // Create statistics collector
    auto statsCollector = std::make_shared<StatisticsCollector>(flows, queue);
    statsCollector->setSampleInterval(100);  // Sample every 100ms
    
    // Start simulation
    std::cout << "Starting simulation...\n";
    generator->start();
    shaper->start();
    statsCollector->start();
    
    // Run for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Stop simulation
    std::cout << "Stopping simulation...\n";
    generator->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    shaper->stop();
    statsCollector->stop();
    queue->shutdown();
    
    // Print and save statistics
    statsCollector->printSummary();
    statsCollector->saveToCSV("results/scenario1_stats.csv");
    std::cout << "Statistics saved to: results/scenario1_stats.csv\n";
    std::cout << "Run: python visualize.py results/scenario1_stats.csv\n";
}

void runScenario2() {
    std::cout << "\n========== Scenario 2: Priority-Based QoS ==========\n";
    std::cout << "Testing QoS with different priority flows\n";
    std::cout << "Observing priority-based packet scheduling\n\n";

    // Network parameters
    uint64_t linkCapacity = 10 * 1000000;  // 10 Mbps
    uint64_t tokenRate = 600 * 1024;       // 600 KB/s
    uint64_t bucketSize = 80 * 1024;       // 80 KB
    size_t queueSize = 400;

    printConfiguration(linkCapacity, tokenRate, bucketSize, queueSize);

    auto queue = std::make_shared<PacketQueue>(queueSize);
    auto tokenBucket = std::make_shared<TokenBucket>(tokenRate, bucketSize);
    
    // Create flows with different priorities
    auto flow1 = std::make_shared<Flow>(1, FlowType::CONSTANT_RATE, 
                                        300 * 1024, PacketPriority::HIGH);
    auto flow2 = std::make_shared<Flow>(2, FlowType::CONSTANT_RATE, 
                                        300 * 1024, PacketPriority::MEDIUM);
    auto flow3 = std::make_shared<Flow>(3, FlowType::CONSTANT_RATE, 
                                        300 * 1024, PacketPriority::LOW);
    
    std::vector<std::shared_ptr<Flow>> flows = {flow1, flow2, flow3};
    
    std::cout << "Flows:\n";
    std::cout << "  Flow 1: 300 KB/s (HIGH Priority)\n";
    std::cout << "  Flow 2: 300 KB/s (MEDIUM Priority)\n";
    std::cout << "  Flow 3: 300 KB/s (LOW Priority)\n\n";

    auto generator = std::make_shared<TrafficGenerator>(queue);
    for (const auto& flow : flows) {
        generator->addFlow(flow);
    }
    
    auto shaper = std::make_shared<TrafficShaper>(queue, tokenBucket, linkCapacity);
    for (const auto& flow : flows) {
        shaper->addFlow(flow);
    }
    
    auto statsCollector = std::make_shared<StatisticsCollector>(flows, queue);
    statsCollector->setSampleInterval(100);
    
    std::cout << "Starting simulation...\n";
    generator->start();
    shaper->start();
    statsCollector->start();
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    std::cout << "Stopping simulation...\n";
    generator->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    shaper->stop();
    statsCollector->stop();
    queue->shutdown();
    
    statsCollector->printSummary();
    statsCollector->saveToCSV("results/scenario2_stats.csv");
    std::cout << "Statistics saved to: results/scenario2_stats.csv\n";
    std::cout << "Run: python visualize.py results/scenario2_stats.csv\n";
}

void runScenario3() {
    std::cout << "\n========== Scenario 3: Bursty Traffic Handling ==========\n";
    std::cout << "Testing TBF with mix of bursty and constant flows\n";
    std::cout << "Observing congestion control and buffer management\n\n";

    // Network parameters
    uint64_t linkCapacity = 10 * 1000000;  // 10 Mbps
    uint64_t tokenRate = 700 * 1024;       // 700 KB/s
    uint64_t bucketSize = 150 * 1024;      // 150 KB (larger for bursts)
    size_t queueSize = 600;

    printConfiguration(linkCapacity, tokenRate, bucketSize, queueSize);

    auto queue = std::make_shared<PacketQueue>(queueSize);
    auto tokenBucket = std::make_shared<TokenBucket>(tokenRate, bucketSize);
    
    // Mix of flow types
    auto flow1 = std::make_shared<Flow>(1, FlowType::BURSTY, 
                                        400 * 1024, PacketPriority::MEDIUM);
    auto flow2 = std::make_shared<Flow>(2, FlowType::CONSTANT_RATE, 
                                        300 * 1024, PacketPriority::MEDIUM);
    auto flow3 = std::make_shared<Flow>(3, FlowType::POISSON, 
                                        350 * 1024, PacketPriority::MEDIUM);
    
    std::vector<std::shared_ptr<Flow>> flows = {flow1, flow2, flow3};
    
    std::cout << "Flows:\n";
    std::cout << "  Flow 1: 400 KB/s (BURSTY)\n";
    std::cout << "  Flow 2: 300 KB/s (CONSTANT_RATE)\n";
    std::cout << "  Flow 3: 350 KB/s (POISSON)\n\n";

    auto generator = std::make_shared<TrafficGenerator>(queue);
    for (const auto& flow : flows) {
        generator->addFlow(flow);
    }
    
    auto shaper = std::make_shared<TrafficShaper>(queue, tokenBucket, linkCapacity);
    for (const auto& flow : flows) {
        shaper->addFlow(flow);
    }
    
    auto statsCollector = std::make_shared<StatisticsCollector>(flows, queue);
    statsCollector->setSampleInterval(100);
    
    std::cout << "Starting simulation...\n";
    generator->start();
    shaper->start();
    statsCollector->start();
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    std::cout << "Stopping simulation...\n";
    generator->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    shaper->stop();
    statsCollector->stop();
    queue->shutdown();
    
    statsCollector->printSummary();
    statsCollector->saveToCSV("results/scenario3_stats.csv");
    std::cout << "Statistics saved to: results/scenario3_stats.csv\n";
    std::cout << "Run: python visualize.py results/scenario3_stats.csv\n";
}

int main(int argc, char* argv[]) {
    printBanner();
    
    int scenario = 0;
    
    if (argc > 1) {
        scenario = std::atoi(argv[1]);
    } else {
        std::cout << "Select a scenario to run:\n";
        std::cout << "  1. Basic Traffic Shaping (3 constant flows)\n";
        std::cout << "  2. Priority-Based QoS (different priorities)\n";
        std::cout << "  3. Bursty Traffic Handling (mixed traffic types)\n";
        std::cout << "  4. Run all scenarios\n";
        std::cout << "\nEnter scenario number (1-4): ";
        std::cin >> scenario;
    }
    
    switch (scenario) {
        case 1:
            runScenario1();
            break;
        case 2:
            runScenario2();
            break;
        case 3:
            runScenario3();
            break;
        case 4:
            runScenario1();
            std::cout << "\n\n";
            runScenario2();
            std::cout << "\n\n";
            runScenario3();
            break;
        default:
            std::cout << "Invalid scenario number. Please choose 1-4.\n";
            return 1;
    }
    
    std::cout << "\n========== Simulation Complete ==========\n";
    std::cout << "To visualize results, run:\n";
    std::cout << "  python visualize.py <csv_file>\n";
    std::cout << "\nExample:\n";
    std::cout << "  python visualize.py results/scenario1_stats.csv\n\n";
    
    return 0;
}
