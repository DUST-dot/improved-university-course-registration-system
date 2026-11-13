#include "recommendation_service.hpp"

#include <algorithm>
#include <set>

namespace course_selection {

OperationResult RecommendationService::recommend(int student_id, std::size_t limit,
                                                 std::vector<Recommendation>& result) const {
    result.clear();
    if (context_.find_student(student_id) == nullptr) {
        return {ResultCode::RecordNotFound, "Student not found"};
    }

    std::set<int> taken;
    for (const Enrollment& record : context_.active_enrollments(student_id)) {
        taken.insert(record.teaching_class_id);
    }

    for (const TeachingClass& item : context_.teaching_classes()) {
        if (taken.count(item.teaching_class_id) != 0) continue;          // already enrolled
        const int seats = context_.remaining_seats(item.teaching_class_id);
        if (seats <= 0) continue;                                        // full

        Course course{};
        if (!context_.find_course(item.course_id, course)) continue;

        Recommendation entry{};
        entry.teaching_class_id = item.teaching_class_id;
        entry.course_code = course.course_code;
        if (seats >= 10) {
            entry.score += 3;
            entry.reason += "plenty of seats; ";
        } else if (seats >= 3) {
            entry.score += 1;
            entry.reason += "some seats left; ";
        }
        if (course.credits <= 3.0) {
            entry.score += 2;
            entry.reason += "light workload; ";
        }
        if (entry.reason.empty()) entry.reason = "available";
        result.push_back(entry);
    }

    std::sort(result.begin(), result.end(),
              [](const Recommendation& a, const Recommendation& b) {
                  if (a.score != b.score) return a.score > b.score;
                  return a.course_code < b.course_code;
              });
    if (result.size() > limit) result.resize(limit);
    return {ResultCode::Success, result.empty() ? "Nothing to recommend" : "Recommendations ready"};
}

}  // namespace course_selection
