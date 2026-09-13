#include "air/microsoc_runtime.hpp"

#include "air/q31.hpp"

namespace air {

MicroSoCRuntime::MicroSoCRuntime(AudioFormat format, MicroSoCProfile profile)
    : format_(format), profile_(profile) {}

std::optional<FramePlan> MicroSoCRuntime::process(AudioFrame& frame, int32_t q31Gain,
                                                   bool requestAnalysis) {
    const auto samplesPerChannel = format_.frameSamplesPerChannel();
    const uint32_t expectedSamples = samplesPerChannel.value_or(0) * format_.channels;
    if (!format_.isValidForMicroSoC() || frame.sampleCount != expectedSamples ||
        frame.sampleCount > kMaxFrameSamples) {
        ++stats_.rejectedFrames;
        return std::nullopt;
    }

    // Every critical stage consumes part of the same codec-frame deadline.
    // Checking each stage against the original budget would accept a pipeline
    // whose aggregate WCET cannot meet its presentation time.
    uint32_t remainingBudgetUs = format_.frameDurationUs;
    FramePlan plan{};
    const auto scheduleCritical = [&](WorkKind kind) {
        const WorkPlacement placement =
            chooseWorkPlacement(profile_, {kind, remainingBudgetUs, false});
        if (placement == WorkPlacement::CpuDsp) {
            remainingBudgetUs -= profile_.cpuWorstCaseUs + profile_.dmaGuardUs;
        }
        return placement;
    };
    plan.preprocess = scheduleCritical(WorkKind::Preprocess);
    plan.codecTransform = scheduleCritical(WorkKind::CodecTransform);
    plan.quantize = scheduleCritical(WorkKind::Quantize);
    plan.packetize = scheduleCritical(WorkKind::Packetize);

    // Analysis is deliberately best-effort. It may use the GPU only after
    // the real-time codec path has reserved its full worst-case budget.
    plan.analysis = requestAnalysis
        ? chooseWorkPlacement(profile_, {WorkKind::Analysis, remainingBudgetUs, true})
        : WorkPlacement::Rejected;
    if (plan.preprocess == WorkPlacement::Rejected || plan.codecTransform == WorkPlacement::Rejected ||
        plan.quantize == WorkPlacement::Rejected || plan.packetize == WorkPlacement::Rejected) {
        ++stats_.rejectedFrames;
        return std::nullopt;
    }

    for (uint16_t index = 0; index < frame.sampleCount; ++index) {
        frame.q31Samples[index] = multiplyQ31(frame.q31Samples[index], q31Gain);
    }
    ++stats_.processedFrames;
    if (plan.analysis == WorkPlacement::GpuCompute) ++stats_.bestEffortGpuFrames;
    return plan;
}

}  // namespace air
