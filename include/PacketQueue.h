#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include "Packet.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>

// Custom comparator for priority queue
struct PacketComparator {
    bool operator()(const std::shared_ptr<Packet>& a, const std::shared_ptr<Packet>& b) const {
        // Higher priority value = higher priority (processed first)
        if (a->getPriority() != b->getPriority()) {
            return a->getPriority() < b->getPriority();
        }
        // For same priority, FIFO (earlier creation time first)
        return a->getCreationTime() > b->getCreationTime();
    }
};

class PacketQueue {
public:
    PacketQueue(size_t maxSize = 1000)
        : maxSize_(maxSize)
        , currentSize_(0)
        , totalDropped_(0)
        , shutdown_(false) {}

    // Enqueue a packet (returns false if queue is full)
    bool enqueue(std::shared_ptr<Packet> packet) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (currentSize_ >= maxSize_) {
            totalDropped_++;
            return false;  // Queue full, packet dropped
        }
        
        queue_.push(packet);
        currentSize_++;
        cv_.notify_one();
        return true;
    }

    // Dequeue a packet (blocks if queue is empty)
    std::shared_ptr<Packet> dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
        
        if (shutdown_ && queue_.empty()) {
            return nullptr;
        }
        
        auto packet = queue_.top();
        queue_.pop();
        currentSize_--;
        return packet;
    }

    // Try to dequeue without blocking
    std::shared_ptr<Packet> tryDequeue() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return nullptr;
        }
        
        auto packet = queue_.top();
        queue_.pop();
        currentSize_--;
        return packet;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return currentSize_;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t getTotalDropped() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return totalDropped_;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }

private:
    std::priority_queue<std::shared_ptr<Packet>, 
                       std::vector<std::shared_ptr<Packet>>,
                       PacketComparator> queue_;
    size_t maxSize_;
    size_t currentSize_;
    size_t totalDropped_;
    bool shutdown_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // PACKET_QUEUE_H
