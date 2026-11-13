#include "monitor_service.hpp"

namespace course_selection {

OperationResult MonitorService::monitor(MonitorSnapshot& result) const {
    result = MonitorSnapshot{};

    for (const TeachingClass& item : context_.teaching_classes()) {
        ++result.teaching_classes;
        // Same seat calculation the student sees, so the numbers cannot disagree.
        if (context_.remaining_seats(item.teaching_class_id) <= 0) ++result.full_classes;
    }
    for (const Enrollment& record : context_.all_enrollments()) {
        if (record.status == EnrollmentStatus::Active) ++result.active_enrollments;
    }

    if (result.teaching_classes > 0 &&
        static_cast<double>(result.full_classes) / result.teaching_classes >
            kFullRatioAlarm) {
        result.alarms.push_back("More than 80% of the teaching classes are full");
    }
    if (result.alarms.empty()) result.alarms.push_back("No alarm");
    return {ResultCode::Success, "Monitor snapshot ready"};
}

}  // namespace course_selection
