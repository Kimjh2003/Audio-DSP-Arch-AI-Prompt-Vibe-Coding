#pragma once

#include <cstdint>

namespace air {

enum class WorkKind { Preprocess, CodecTransform, Quantize, Packetize, Analysis };
enum class WorkPlacement { CpuDsp, GpuCompute, Rejected };

struct MicroSoCProfile {
    bool hasDma = true;
    bool hasSimd = false;
    bool hasGpuCompute = false;
    uint32_t cpuWorstCaseUs = 0;
    uint32_t gpuSubmitAndRunWorstCaseUs = 0;
    uint32_t dmaGuardUs = 0;
};

struct WorkRequest {
    WorkKind kind = WorkKind::Preprocess;
    uint32_t remainingDeadlineUs = 0;
    bool batchable = false;
};

// Real-time codec work stays on the CPU/DSP. GPU work is only selected for
// explicitly batchable analysis when its full submit/sync cost fits the budget.
WorkPlacement chooseWorkPlacement(const MicroSoCProfile& profile, const WorkRequest& request);

}  // namespace air

