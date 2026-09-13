#pragma once

#include <cstdint>
#include <limits>

namespace air {

inline int32_t saturateQ31(int64_t value) {
    if (value > std::numeric_limits<int32_t>::max()) return std::numeric_limits<int32_t>::max();
    if (value < std::numeric_limits<int32_t>::min()) return std::numeric_limits<int32_t>::min();
    return static_cast<int32_t>(value);
}

inline int32_t multiplyQ31(int32_t left, int32_t right) {
    constexpr int64_t kRound = 1LL << 30;
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    return saturateQ31((product + kRound) >> 31);
}

}  // namespace air
