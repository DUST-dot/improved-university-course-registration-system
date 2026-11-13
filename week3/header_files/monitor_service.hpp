#pragma once
// 2.14 US-17 Real-time monitoring.
// Read-only aggregate: it reports, it never changes business data.
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct MonitorSnapshot {
    int teaching_classes{0};
    int full_classes{0};
    int active_enrollments{0};
    std::vector<std::string> alarms;
};

class MonitorService {
public:
    explicit MonitorService(const SystemContext& context) : context_(context) {}

    // Thresholds live here so a rule change touches one line.
    static constexpr double kFullRatioAlarm = 0.8;

    OperationResult monitor(MonitorSnapshot& result) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
