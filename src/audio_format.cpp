#include "air/audio_format.hpp"

namespace air {

std::optional<uint32_t> AudioFormat::frameSamplesPerChannel() const {
    const uint64_t numerator = static_cast<uint64_t>(sampleRateHz) * frameDurationUs;
    if (sampleRateHz == 0 || frameDurationUs == 0 || numerator % 1'000'000 != 0) return std::nullopt;
    return static_cast<uint32_t>(numerator / 1'000'000);
}

bool AudioFormat::isValidForMicroSoC() const {
    const auto samples = frameSamplesPerChannel();
    if (!samples.has_value() || (frameDurationUs != 7500 && frameDurationUs != 10000)) return false;
    if (channels == 0 || channels > 2 || representation == SampleRepresentation::Float32) return false;
    return *samples * channels <= 960;
}

}  // namespace air

