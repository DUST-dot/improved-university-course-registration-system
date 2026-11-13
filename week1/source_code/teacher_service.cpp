#include "teacher_service.hpp"

#include <algorithm>

#include "authentication_service.hpp"
#include "permission_rules.hpp"

namespace course_selection {

TeacherService::TeacherService(const SystemContext& context) : context_(context) {}

TeachingClassSummary TeacherService::build_summary(const TeachingClass& item) const {
    TeachingClassSummary summary{};
    summary.teaching_class_id = item.teaching_class_id;
    summary.course_id = item.course_id;

    Course course{};
    if (context_.find_course(item.course_id, course)) {
        summary.course_code = course.course_code;
        summary.course_name = course.course_name;
        summary.credits = course.credits;
    }
    summary.teacher_id = item.teacher_id;
    const Teacher* teacher = context_.find_teacher(item.teacher_id);
    summary.teacher_name = teacher == nullptr ? std::string() : teacher->name;
    summary.capacity = item.capacity;
    // 人数与余量都现算：和学生端看到的是同一批记录，不可能对不上
    summary.active_enrollments = context_.enrollment_count(item.teaching_class_id);
    summary.remaining_seats = context_.remaining_seats(item.teaching_class_id);
    summary.meetings = item.meetings;
    return summary;
}

OperationResult TeacherService::assigned_teaching_classes(
    const UserIdentity& user, std::vector<TeachingClassSummary>& result) const {
    result.clear();

    // 入口守卫：学生账号在这里就被挡住，不用等后面过滤
    const OperationResult guard = AuthenticationService::require_role(user, Role::Teacher);
    if (!PermissionRules::is_administrator(user) && guard.code != ResultCode::Success) {
        return guard;
    }

    for (const TeachingClass& item : context_.teaching_classes()) {
        // 教师只列本人的班；管理员可列全部
        if (!PermissionRules::can_view_teaching_class(user, item)) continue;
        result.push_back(build_summary(item));
    }
    std::sort(result.begin(), result.end(),
              [](const TeachingClassSummary& left, const TeachingClassSummary& right) {
                  return left.teaching_class_id < right.teaching_class_id;
              });
    return {ResultCode::Success, "Assigned teaching classes listed"};
}

OperationResult TeacherService::class_summary(const UserIdentity& user,
                                              int teaching_class_id,
                                              TeachingClassSummary& result) const {
    result = TeachingClassSummary{};

    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::RecordNotFound, "Teaching class not found"};
    }
    // 即使 id 猜对也读不到别人的班
    if (!PermissionRules::can_view_teaching_class(user, item)) {
        return {ResultCode::PermissionDenied, "You can only view your own teaching classes"};
    }
    result = build_summary(item);
    return {ResultCode::Success, "Class summary ready"};
}

OperationResult TeacherService::class_roster(const UserIdentity& user,
                                             int teaching_class_id,
                                             ClassRoster& result) const {
    result = ClassRoster{};

    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::RecordNotFound, "Teaching class not found"};
    }
    if (!PermissionRules::can_view_teaching_class(user, item)) {
        return {ResultCode::PermissionDenied, "You can only view your own teaching classes"};
    }

    result.teaching_class_id = item.teaching_class_id;
    result.capacity = item.capacity;
    result.active_enrollments = context_.enrollment_count(teaching_class_id);
    result.remaining_seats = context_.remaining_seats(teaching_class_id);

    for (const Enrollment& record : context_.all_enrollments()) {
        if (record.teaching_class_id != teaching_class_id) continue;
        if (record.status != EnrollmentStatus::Active) continue;   // 退课不出现
        const Student* student = context_.find_student(record.student_id);
        if (student == nullptr) continue;
        result.students.push_back({student->student_id, student->student_number, student->name});
    }
    // 按学号排序，保证同样的数据每次输出一致
    std::sort(result.students.begin(), result.students.end(),
              [](const RosterEntry& left, const RosterEntry& right) {
                  return left.student_number < right.student_number;
              });
    return {ResultCode::Success, "Roster ready"};
}

}  // namespace course_selection
