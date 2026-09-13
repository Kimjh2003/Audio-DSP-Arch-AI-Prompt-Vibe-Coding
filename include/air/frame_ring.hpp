#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

namespace air {

// Single-producer/single-consumer ring for DMA callback -> audio worker.
// Capacity must be a power of two and no allocation occurs after construction.
template <typename T, size_t Capacity>
class FrameRing {
    static_assert(Capacity > 1 && (Capacity & (Capacity - 1)) == 0);

public:
    bool push(const T& value) {
        const size_t write = writeIndex_.load(std::memory_order_relaxed);
        const size_t next = (write + 1) & (Capacity - 1);
        if (next == readIndex_.load(std::memory_order_acquire)) return false;
        slots_[write] = value;
        writeIndex_.store(next, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        const size_t read = readIndex_.load(std::memory_order_relaxed);
        if (read == writeIndex_.load(std::memory_order_acquire)) return std::nullopt;
        T value = slots_[read];
        readIndex_.store((read + 1) & (Capacity - 1), std::memory_order_release);
        return value;
    }

private:
    std::array<T, Capacity> slots_{};
    std::atomic<size_t> readIndex_{0};
    std::atomic<size_t> writeIndex_{0};
};

}  // namespace air
