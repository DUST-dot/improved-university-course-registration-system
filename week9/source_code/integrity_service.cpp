#include "integrity_service.hpp"

#include <set>

namespace course_selection {

OperationResult IntegrityService::check(std::vector<std::string>& problems) const {
    problems.clear();
    std::set<std::pair<int, int>> seen;   // (student, class) already counted

    for (const Enrollment& record : context_.all_enrollments()) {
        if (record.status != EnrollmentStatus::Active) continue;

        if (!seen.insert({record.student_id, record.teaching_class_id}).second) {
            problems.push_back("Duplicate active enrollment: student " +
                               std::to_string(record.student_id) + " / class " +
                               std::to_string(record.teaching_class_id));
        }
        if (context_.find_student(record.student_id) == nullptr) {
            problems.push_back("Enrollment points at a missing student: " +
                               std::to_string(record.student_id));
        }
        TeachingClass item{};
        if (!context_.find_teaching_class(record.teaching_class_id, item)) {
            problems.push_back("Enrollment points at a missing class: " +
                               std::to_string(record.teaching_class_id));
        }
    }

    for (const TeachingClass& item : context_.teaching_classes()) {
        const int active = context_.enrollment_count(item.teaching_class_id);
        if (active > item.capacity) {
            problems.push_back("Over capacity: class " +
                               std::to_string(item.teaching_class_id) + " has " +
                               std::to_string(active) + " of " +
                               std::to_string(item.capacity));
        }
    }

    return {problems.empty() ? ResultCode::Success : ResultCode::InvalidInput,
            problems.empty() ? "Data is consistent" : "Problems found"};
}

void IntegrityService::resync() {
    // The seat count is derived, so a resync is a no-op: this is the point.
    // If a counter were cached, this is where it would be rebuilt.
}

}  // namespace course_selection
