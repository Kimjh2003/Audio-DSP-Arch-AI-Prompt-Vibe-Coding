#include "air/realtime_scheduler.hpp"

namespace air {

WorkPlacement chooseWorkPlacement(const MicroSoCProfile& profile, const WorkRequest& request) {
    const bool codecCritical = request.kind == WorkKind::Preprocess ||
        request.kind == WorkKind::CodecTransform || request.kind == WorkKind::Quantize ||
        request.kind == WorkKind::Packetize;
    if (codecCritical) {
        return profile.hasDma && profile.cpuWorstCaseUs + profile.dmaGuardUs <= request.remainingDeadlineUs
            ? WorkPlacement::CpuDsp : WorkPlacement::Rejected;
    }
    if (request.kind == WorkKind::Analysis && request.batchable && profile.hasGpuCompute &&
        profile.gpuSubmitAndRunWorstCaseUs + profile.dmaGuardUs <= request.remainingDeadlineUs) {
        return WorkPlacement::GpuCompute;
    }
    return profile.cpuWorstCaseUs + profile.dmaGuardUs <= request.remainingDeadlineUs
        ? WorkPlacement::CpuDsp : WorkPlacement::Rejected;
}

}  // namespace air
