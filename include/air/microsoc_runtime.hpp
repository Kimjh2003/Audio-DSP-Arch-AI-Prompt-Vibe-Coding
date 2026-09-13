#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "air/audio_format.hpp"
#include "air/realtime_scheduler.hpp"

namespace air {

constexpr uint32_t kMaxFrameSamples = 960;  // 48 kHz, 10 ms, stereo.

struct AudioFrame {
    uint64_t presentationTimeUs = 0;
    uint16_t sampleCount = 0;
    std::array<int32_t, kMaxFrameSamples> q31Samples{};
};

struct FramePlan {
    WorkPlacement preprocess = WorkPlacement::CpuDsp;
    WorkPlacement codecTransform = WorkPlacement::CpuDsp;
    WorkPlacement quantize = WorkPlacement::CpuDsp;
    WorkPlacement packetize = WorkPlacement::CpuDsp;
    WorkPlacement analysis = WorkPlacement::Rejected;
};

struct RuntimeStats {
    uint64_t processedFrames = 0;
    uint64_t rejectedFrames = 0;
    uint64_t bestEffortGpuFrames = 0;
};

class MicroSoCRuntime {
public:
    MicroSoCRuntime(AudioFormat format, MicroSoCProfile profile);

    [[nodiscard]] std::optional<FramePlan> process(AudioFrame& frame, int32_t q31Gain,
                                                    bool requestAnalysis);
    [[nodiscard]] const RuntimeStats& stats() const { return stats_; }

private:
    AudioFormat format_;
    MicroSoCProfile profile_;
    RuntimeStats stats_{};
};

}  // namespace air

