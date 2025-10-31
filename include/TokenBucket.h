#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include <chrono>
#include <mutex>
#include <cstdint>

class TokenBucket {
public:
    TokenBucket(uint64_t rate, uint64_t bucketSize)
        : rate_(rate)                  // Tokens per second (bytes/sec)
        , bucketSize_(bucketSize)      // Maximum bucket capacity (bytes)
        , tokens_(bucketSize)          // Start with full bucket
        , lastUpdate_(std::chrono::high_resolution_clock::now()) {}

    // Try to consume tokens for a packet
    bool consume(uint32_t tokens) {
        std::lock_guard<std::mutex> lock(mutex_);
        refill();
        
        if (tokens_ >= tokens) {
            tokens_ -= tokens;
            return true;
        }
        return false;
    }

    // Get current token count
    uint64_t getTokens() {
        std::lock_guard<std::mutex> lock(mutex_);
        refill();
        return tokens_;
    }

    uint64_t getRate() const { return rate_; }
    uint64_t getBucketSize() const { return bucketSize_; }

private:
    void refill() {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            now - lastUpdate_).count();
        
        // Calculate tokens to add based on elapsed time
        uint64_t tokensToAdd = (rate_ * elapsed) / 1000000;
        
        if (tokensToAdd > 0) {
            tokens_ = std::min(tokens_ + tokensToAdd, bucketSize_);
            lastUpdate_ = now;
        }
    }

    uint64_t rate_;           // Token generation rate (bytes/sec)
    uint64_t bucketSize_;     // Maximum bucket capacity
    uint64_t tokens_;         // Current token count
    std::chrono::high_resolution_clock::time_point lastUpdate_;
    std::mutex mutex_;
};

#endif // TOKEN_BUCKET_H
