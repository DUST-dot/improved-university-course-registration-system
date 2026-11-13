#include "analytics_service.hpp"

#include <algorithm>
#include <map>

namespace course_selection {

OperationResult AnalyticsService::distribution(
    const std::string& dimension, std::vector<std::pair<std::string, int>>& result) const {
    result.clear();
    if (dimension != "course" && dimension != "college") {
        return {ResultCode::InvalidInput, "Unknown dimension: " + dimension};
    }

    std::map<std::string, int> counter;
    for (const Enrollment& record : context_.all_enrollments()) {
        if (record.status != EnrollmentStatus::Active) continue;   // dropped rows never count

        TeachingClass item{};
        if (!context_.find_teaching_class(record.teaching_class_id, item)) continue;
        Course course{};
        if (!context_.find_course(item.course_id, course)) continue;

        if (dimension == "course") {
            ++counter[course.course_code];
        } else {
            const Teacher* teacher = context_.find_teacher(item.teacher_id);
            ++counter[teacher == nullptr ? "Unknown" : teacher->department];
        }
    }

    result.assign(counter.begin(), counter.end());
    // Descending by count, then ascending by name: stable output for the same data.
    std::sort(result.begin(), result.end(),
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first < b.first;
              });
    return {ResultCode::Success, "Distribution ready"};
}

}  // namespace course_selection
