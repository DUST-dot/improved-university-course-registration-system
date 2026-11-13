#include "performance_service.hpp"

namespace course_selection {

int PerformanceService::count_by_scan(int teaching_class_id) const {
    int count = 0;
    for (const Enrollment& record : context_.all_enrollments()) {
        ++scanned_;
        if (record.teaching_class_id == teaching_class_id &&
            record.status == EnrollmentStatus::Active) {
            ++count;
        }
    }
    return count;
}

void PerformanceService::rebuild_index() {
    index_.clear();
    for (const Enrollment& record : context_.all_enrollments()) {
        if (record.status != EnrollmentStatus::Active) continue;
        ++index_[record.teaching_class_id];
    }
}

int PerformanceService::count_by_index(int teaching_class_id) const {
    ++calls_;
    auto found = index_.find(teaching_class_id);
    return found == index_.end() ? 0 : found->second;
}

}  // namespace course_selection
