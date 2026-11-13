#pragma once

// ============================================================================
// 权限规则（week1 与 2.5–2.9 合并）
//
// 合并后只有这一处判断角色与归属，Service 里不再写 if (role == ...)：
//   教师：只能读本人教学班；可申请开课、提交调课；不能改目录与人员。
//   管理员：可维护目录、容量、选课时段；可读任意教学班；可审批。
//   学生：没有教师入口，只走自己的查询。
// ============================================================================

#include "domain.hpp"

namespace course_selection {

class PermissionRules {
public:
    static bool is_student(const UserIdentity& user) { return user.role == Role::Student; }
    static bool is_teacher(const UserIdentity& user) { return user.role == Role::Teacher; }
    static bool is_administrator(const UserIdentity& user) {
        return user.role == Role::Administrator;
    }

    // week1 / US-09：教师只能看自己的班，管理员可看全部
    static bool can_view_teaching_class(const UserIdentity& user,
                                        const TeachingClass& item) {
        if (is_administrator(user)) return true;
        if (!is_teacher(user)) return false;
        return item.teacher_id == user.user_id;
    }

    // US-13：目录与学分上限
    static bool can_manage_catalog(const UserIdentity& user) {
        return is_administrator(user);
    }

    // US-13 / US-16：选课时段
    static bool can_set_registration_period(const UserIdentity& user) {
        return is_administrator(user);
    }

    static bool can_manage_people(const UserIdentity& user) {
        return is_administrator(user);
    }

    // US-10：开课申请只有教师能提
    static bool can_open_course(const UserIdentity& user) {
        return is_teacher(user);
    }

    // US-14 及后续审批：只有管理员
    static bool can_review_application(const UserIdentity& user) {
        return is_administrator(user);
    }

    // US-12：只能针对自己的教学班提调课 / 停课
    static bool can_submit_change_request(const UserIdentity& user,
                                          const TeachingClass& item) {
        return is_teacher(user) && item.teacher_id == user.user_id;
    }
};

}  // namespace course_selection
