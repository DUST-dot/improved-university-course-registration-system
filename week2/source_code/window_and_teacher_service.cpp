#include "window_and_teacher_service.hpp"

#include <algorithm>

#include "permission_rules.hpp"
#include "time_util.hpp"

namespace course_selection {

// ---------------------------------------------------------------------------
// US-16
// ---------------------------------------------------------------------------

const char* window_state_name(WindowState state) {
    switch (state) {
        case WindowState::NotStarted: return "NotStarted";
        case WindowState::Open:       return "Open";
        case WindowState::Closed:     return "Closed";
    }
    return "Unknown";
}

WindowState state_of(const RegistrationRule& rule, const std::string& now) {
    if (rule.opens_at.empty() || rule.closes_at.empty()) {
        return rule.registration_open ? WindowState::Open : WindowState::Closed;
    }
    if (moment_before(now, rule.opens_at)) return WindowState::NotStarted;
    if (moment_before(now, rule.closes_at)) return WindowState::Open;
    return WindowState::Closed;
}

OperationResult RegistrationWindowService::set_registration_period(
    const UserIdentity& user, const std::string& opens_at, const std::string& closes_at) {
    if (!PermissionRules::can_set_registration_period(user)) {
        return {ResultCode::PermissionDenied, "Only administrators can set the period"};
    }

    long long open_min = 0;
    long long close_min = 0;
    if (!parse_iso_minute(opens_at, open_min) || !parse_iso_minute(closes_at, close_min)) {
        return {ResultCode::InvalidInput, "Time format must be YYYY-MM-DDTHH:MM"};
    }
    if (open_min >= close_min) {
        return {ResultCode::InvalidInput, "The opening time must be earlier than closing"};
    }

    // 与已存在的轮次判交：a_start < b_end && b_start < a_end
    for (const RegistrationRound& existing : context_.registration_rounds()) {
        long long a = 0;
        long long b = 0;
        if (!parse_iso_minute(existing.opens_at, a) ||
            !parse_iso_minute(existing.closes_at, b)) {
            continue;
        }
        const bool same_round =
            existing.opens_at == opens_at && existing.closes_at == closes_at;
        if (same_round) continue;   // 重复提交同一轮次，不算冲突
        if (open_min < b && a < close_min) {
            return {ResultCode::InvalidInput, "Overlaps with an existing round"};
        }
    }

    RegistrationRule rule = context_.rule();
    rule.opens_at = opens_at;
    rule.closes_at = closes_at;
    rule.registration_open = true;
    context_.apply_rule_change(rule);

    // 同一轮次重复提交是允许的（管理员可能只是重设一次），但不该登记两条：
    // 否则轮次列表里会出现两条一模一样的区间，后续判重叠也会多绕一圈。
    const bool already_recorded =
        std::any_of(context_.registration_rounds().begin(),
                    context_.registration_rounds().end(),
                    [&](const RegistrationRound& round) {
                        return round.opens_at == opens_at && round.closes_at == closes_at;
                    });
    if (!already_recorded) {
        context_.add_round(RegistrationRound{opens_at, closes_at});
    }
    return {ResultCode::Success, "Registration period updated"};
}

OperationResult RegistrationWindowService::set_manual_switch(const UserIdentity& user, bool open) {
    if (!PermissionRules::can_set_registration_period(user)) {
        return {ResultCode::PermissionDenied, "Only administrators can switch the window"};
    }
    RegistrationRule rule = context_.rule();
    rule.registration_open = open;
    context_.apply_rule_change(rule);
    return {ResultCode::Success, open ? "Window opened manually" : "Window closed manually"};
}

OperationResult RegistrationWindowService::current_rule(RegistrationRule& result) const {
    result = context_.rule();
    return {ResultCode::Success, "Registration rule ready"};
}

OperationResult RegistrationWindowService::window_state(WindowState& result) const {
    return window_state_at(context_.now(), result);
}

OperationResult RegistrationWindowService::window_state_at(const std::string& now,
                                                           WindowState& result) const {
    result = state_of(context_.rule(), now);
    return {ResultCode::Success, window_state_name(result)};
}

// ---------------------------------------------------------------------------
// US-11（含 US-09）
// ---------------------------------------------------------------------------

TeachingClassSummary TeacherService::build_summary(const TeachingClass& item) const {
    TeachingClassSummary summary{};
    summary.teaching_class_id = item.teaching_class_id;
    summary.course_id = item.course_id;

    Course course{};
    if (context_.find_course(item.course_id, course)) {
        summary.course_code = course.course_code;
        summary.course_name = course.course_name;
    }
    summary.capacity = item.capacity;
    summary.active_enrollments = context_.enrollment_count(item.teaching_class_id);  // 唯一口径
    summary.remaining_seats = context_.remaining_seats(item.teaching_class_id);
    return summary;
}

OperationResult TeacherService::assigned_teaching_classes(
    const UserIdentity& user, std::vector<TeachingClassSummary>& result) const {
    result.clear();
    // 学生没有教师入口；教师只列本人的班；管理员可列全部
    if (!PermissionRules::is_teacher(user) && !PermissionRules::is_administrator(user)) {
        return {ResultCode::PermissionDenied, "This entry requires a different role"};
    }
    for (const TeachingClass& item : context_.teaching_classes()) {
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
    if (!PermissionRules::can_view_teaching_class(user, item)) {
        return {ResultCode::PermissionDenied, "You can only view your own classes"};
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
        return {ResultCode::PermissionDenied, "You can only view your own classes"};
    }

    result.teaching_class_id = item.teaching_class_id;
    for (const Enrollment& record : context_.all_enrollments()) {
        if (record.teaching_class_id != teaching_class_id) continue;
        if (record.status != EnrollmentStatus::Active) continue;   // 退课不出现
        const Student* student = context_.find_student(record.student_id);
        if (student == nullptr) continue;
        result.students.push_back(
            RosterEntry{student->student_id, student->student_number, student->name});
    }
    std::sort(result.students.begin(), result.students.end(),
              [](const RosterEntry& left, const RosterEntry& right) {
                  return left.student_number < right.student_number;
              });
    return {ResultCode::Success, "Roster ready"};
}

}  // namespace course_selection
