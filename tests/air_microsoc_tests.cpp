#include <cassert>
#include <cstdint>

#include "air/audio_format.hpp"
#include "air/frame_ring.hpp"
#include "air/microsoc_runtime.hpp"
#include "air/q31.hpp"

int main() {
    using namespace air;

    const AudioFormat format{48000, 2, 7500, SampleRepresentation::Q31};
    assert(format.isValidForMicroSoC());
    assert(format.frameSamplesPerChannel().value() == 360);
    assert(!AudioFormat{44100, 2, 7500, SampleRepresentation::Q31}.isValidForMicroSoC());

    assert(multiplyQ31(0x40000000, 0x40000000) == 0x20000000);

    FrameRing<int, 4> ring;
    assert(ring.push(1));
    assert(ring.push(2));
    const auto first = ring.pop();
    const auto second = ring.pop();
    assert(first.has_value() && *first == 1);
    assert(second.has_value() && *second == 2);
    assert(!ring.pop().has_value());

    const MicroSoCProfile profile{
        .hasDma = true,
        .hasSimd = true,
        .hasGpuCompute = true,
        .cpuWorstCaseUs = 400,
        .gpuSubmitAndRunWorstCaseUs = 500,
        .dmaGuardUs = 100,
    };
    MicroSoCRuntime runtime(format, profile);
    AudioFrame frame{};
    frame.sampleCount = 720;
    frame.q31Samples[0] = 0x40000000;
    const auto plan = runtime.process(frame, 0x40000000, true);
    assert(plan.has_value());
    assert(plan->codecTransform == WorkPlacement::CpuDsp);
    assert(plan->analysis == WorkPlacement::GpuCompute);
    assert(frame.q31Samples[0] == 0x20000000);

    const MicroSoCProfile tooSlow{true, false, false, 2000, 0, 100};
    MicroSoCRuntime rejectedRuntime(format, tooSlow);
    assert(!rejectedRuntime.process(frame, 0x7fffffff, false).has_value());

    const MicroSoCProfile noDma{false, false, false, 400, 0, 100};
    MicroSoCRuntime noDmaRuntime(format, noDma);
    assert(!noDmaRuntime.process(frame, 0x7fffffff, false).has_value());
}
