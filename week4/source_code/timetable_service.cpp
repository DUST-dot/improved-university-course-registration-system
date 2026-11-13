#include "timetable_service.hpp"

#include <algorithm>

namespace course_selection {

OperationResult TimetableService::for_student(const UserIdentity& user,
                                              std::vector<TimetableEntry>& result) const {
    result.clear();
    if (user.role != Role::Student) {
        return {ResultCode::PermissionDenied, "Only students have a timetable"};
    }
    if (context_.find_student(user.user_id) == nullptr) {
        return {ResultCode::RecordNotFound, "Student not found"};
    }

    for (const Enrollment& record : context_.active_enrollments(user.user_id)) {
        TeachingClass item{};
        if (!context_.find_teaching_class(record.teaching_class_id, item)) continue;
        Course course{};
        context_.find_course(item.course_id, course);
        const Teacher* teacher = context_.find_teacher(item.teacher_id);

        for (const ClassMeeting& meeting : item.meetings) {
            TimetableEntry entry{};
            entry.teaching_class_id = item.teaching_class_id;
            entry.course_code = course.course_code;
            entry.course_name = course.course_name;
            entry.teacher_name = teacher == nullptr ? std::string() : teacher->name;
            entry.meeting = meeting;
            result.push_back(entry);
        }
    }

    std::sort(result.begin(), result.end(),
              [](const TimetableEntry& a, const TimetableEntry& b) {
                  if (a.meeting.day_of_week != b.meeting.day_of_week) {
                      return a.meeting.day_of_week < b.meeting.day_of_week;
                  }
                  return a.meeting.start_minute < b.meeting.start_minute;
              });
    // An empty timetable is success, not an error: the UI shows "not enrolled yet".
    return {ResultCode::Success,
            result.empty() ? "No course enrolled yet" : "Timetable ready"};
}

}  // namespace course_selection
