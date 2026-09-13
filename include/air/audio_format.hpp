#pragma once

#include <cstdint>
#include <optional>

namespace air {

enum class SampleRepresentation { PcmS16, Q31, Float32 };

struct AudioFormat {
    uint32_t sampleRateHz = 48000;
    uint8_t channels = 2;
    uint32_t frameDurationUs = 10000;
    SampleRepresentation representation = SampleRepresentation::Q31;

    [[nodiscard]] std::optional<uint32_t> frameSamplesPerChannel() const;
    [[nodiscard]] bool isValidForMicroSoC() const;
};

}  // namespace air
