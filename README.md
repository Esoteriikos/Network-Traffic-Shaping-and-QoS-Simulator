# Network Traffic Shaping and QoS Simulator

A comprehensive network traffic simulator implementing **Token Bucket Filter (TBF)** for rate limiting and Quality of Service (QoS) experiments. Built with multithreaded C++ to simulate concurrent traffic flows with realistic packet scheduling, delay modeling, and packet drop policies.

## Features

- **Token Bucket Filter (TBF)** implementation for traffic shaping
- **Multithreaded traffic generation** simulating concurrent network flows
- **Priority-based QoS scheduling** with configurable priority levels
- **Multiple traffic patterns**: Constant-rate, Bursty, and Poisson arrivals
- **Comprehensive statistics collection**: throughput, delay, drop rate, queue occupancy
- **Visualization tools** generating detailed graphs and dashboards
- **Fairness analysis** using Jain's Fairness Index

## Requirements

### Compilation
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher
- pthread library (Linux/Mac)

### Visualization
- Python 3.6+
- Required packages:
  ```bash
  pip install pandas matplotlib numpy
  ```

## Build Instructions

### Windows (Visual Studio)
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Windows (MinGW)
```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

### Linux/Mac
```bash
mkdir build
cd build
cmake ..
make
```

The executable will be created in `build/bin/network_sim`

## 🚀 Usage

### Running Simulations

```bash
# Interactive mode
./build/bin/network_sim

# Run specific scenario
./build/bin/network_sim 1  # Scenario 1
./build/bin/network_sim 2  # Scenario 2
./build/bin/network_sim 3  # Scenario 3
./build/bin/network_sim 4  # Run all scenarios
```

### Scenarios

**Scenario 1: Basic Traffic Shaping**
- 3 constant-rate flows competing for bandwidth
- Demonstrates basic TBF operation and flow fairness
- Token rate: 800 KB/s, Bucket: 100 KB

**Scenario 2: Priority-Based QoS**
- 3 flows with different priorities (HIGH, MEDIUM, LOW)
- Shows priority scheduling and differential treatment
- Token rate: 600 KB/s, Bucket: 80 KB

**Scenario 3: Bursty Traffic Handling**
- Mix of traffic types: Bursty, Constant, Poisson
- Tests congestion control and buffer management
- Token rate: 700 KB/s, Bucket: 150 KB (larger for bursts)

### Generating Visualizations

After running a simulation:

```bash
# Visualize specific scenario
python visualize.py scenario1_stats.csv

# Specify output directory
python visualize.py scenario1_stats.csv my_plots/
```

Generated plots:
- `throughput.png` - Per-flow and aggregate throughput over time
- `queue_occupancy.png` - Queue buffer utilization
- `delay.png` - Average packet delay per flow
- `drop_rate.png` - Packet drop rates
- `fairness.png` - Jain's Fairness Index
- `dashboard.png` - Comprehensive view of all metrics

## 📊 Architecture

```
┌─────────────────┐
│ Traffic         │
│ Generator       │──┐
│ (Multi-threaded)│  │
└─────────────────┘  │
                     ▼
              ┌──────────────┐
              │ Packet Queue │
              │ (Priority)   │
              └──────────────┘
                     │
                     ▼
              ┌──────────────┐      ┌──────────────┐
              │ Traffic      │◄─────│ Token Bucket │
              │ Shaper       │      │ Filter (TBF) │
              └──────────────┘      └──────────────┘
                     │
                     ▼
              ┌──────────────┐
              │ Statistics   │
              │ Collector    │
              └──────────────┘
```

### Key Components

- **Packet**: Network packet with timestamp, size, priority, and flow ID
- **Flow**: Traffic source with configurable rate and traffic pattern
- **TokenBucket**: TBF implementation for rate limiting
- **PacketQueue**: Priority queue with configurable capacity
- **TrafficGenerator**: Multithreaded packet generation
- **TrafficShaper**: Token bucket-based traffic shaping
- **StatisticsCollector**: Real-time metrics collection and CSV export

## 🔬 Key Concepts Demonstrated

### Token Bucket Filter (TBF)
- Tokens generated at constant rate (token rate)
- Bucket has maximum capacity (bucket size)
- Packets consume tokens equal to their size
- Packets wait if insufficient tokens available
- Allows burst transmission up to bucket capacity

### QoS Features
- **Priority Scheduling**: Higher priority packets processed first
- **Queue Management**: Tail-drop policy when queue is full
- **Flow Isolation**: Per-flow statistics and monitoring
- **Fairness**: Measured using Jain's Fairness Index

### Traffic Patterns
- **Constant Rate**: Fixed inter-arrival times
- **Bursty**: Alternating high/low rate periods
- **Poisson**: Exponentially distributed arrivals

## Metrics Collected

- **Throughput**: Bytes transmitted per second (per-flow and aggregate)
- **Delay**: End-to-end packet delay in milliseconds
- **Drop Rate**: Percentage of packets dropped due to queue overflow
- **Queue Occupancy**: Number of packets waiting in queue
- **Fairness Index**: Jain's index measuring bandwidth sharing fairness

## Customization

You can modify simulation parameters in `src/main.cpp`:

```cpp
uint64_t linkCapacity = 10 * 1000000;  // Link speed (bps)
uint64_t tokenRate = 800 * 1024;       // Token rate (bytes/s)
uint64_t bucketSize = 100 * 1024;      // Bucket size (bytes)
size_t queueSize = 500;                // Max queue size (packets)
```

Create custom flows:
```cpp
auto flow = std::make_shared<Flow>(
    flowId,              // Flow identifier
    FlowType::BURSTY,    // Traffic pattern
    400 * 1024,          // Target rate (bytes/s)
    PacketPriority::HIGH // Priority level
);
```

## Project Structure

```
Network simulator/
├── include/
│   ├── Packet.h              # Packet data structure
│   ├── Flow.h                # Traffic flow abstraction
│   ├── TokenBucket.h         # TBF implementation
│   ├── PacketQueue.h         # Priority queue
│   ├── TrafficGenerator.h    # Multithreaded traffic generator
│   ├── TrafficShaper.h       # Traffic shaping engine
│   └── StatisticsCollector.h # Metrics collection
├── src/
│   └── main.cpp              # Main simulation scenarios
├── CMakeLists.txt            # Build configuration
├── visualize.py              # Python visualization script
└── README.md                 # This file
```

## Learning Outcomes

This simulator demonstrates:

1. **Network Traffic Management**: Token Bucket algorithm for rate limiting
2. **Concurrent Programming**: Thread-safe queue and flow management
3. **QoS Mechanisms**: Priority scheduling and differentiated service
4. **Performance Analysis**: Throughput, latency, and fairness metrics
5. **Congestion Control**: Queue management and packet dropping
6. **Data Visualization**: Converting raw metrics to insights

## Example Output

```
========== Simulation Summary ==========
Duration: 10.015 seconds
Total Packets Transmitted: 15834
Total Bytes Transmitted: 7891245
Average Aggregate Throughput: 788.12 KB/s

Per-Flow Statistics:
  FlowID         Sent     Dropped   DropRate%  Throughput(KB/s)   AvgDelay(ms)
--------------------------------------------------------------------------
       1         5432          45        0.83            262.45          12.340
       2         5389          38        0.70            262.78          12.521
       3         5413          51        0.94            262.89          12.445
========================================
```

## Contributing

Feel free to:
- Add new traffic patterns
- Implement additional QoS algorithms (e.g., Weighted Fair Queuing)
- Add more visualization types
- Create additional test scenarios

## License

This is an educational project for learning network traffic management and QoS concepts.

## References

- Token Bucket Algorithm: RFC 2697, RFC 2698
- Jain's Fairness Index: "A Quantitative Measure of Fairness"
- QoS in IP Networks: RFC 2475 (DiffServ)

