#pragma once

// Every role and ownership decision lives here, so no service writes its own
// if (role == ...) chain.
//
//   teacher      owns their classes, may apply to open a course, may file a change
//   administrator can manage catalogue, caps, windows and people; may read any class
//   student      no teacher entry at all

#include "domain.hpp"

namespace course_selection {

class PermissionRules {
public:
    static bool is_student(const UserIdentity& user) { return user.role == Role::Student; }
    static bool is_teacher(const UserIdentity& user) { return user.role == Role::Teacher; }
    static bool is_administrator(const UserIdentity& user) {
        return user.role == Role::Administrator;
    }

    static bool can_view_teaching_class(const UserIdentity& user,
                                        const TeachingClass& item) {
        if (is_administrator(user)) return true;
        if (!is_teacher(user)) return false;
        return item.teacher_id == user.user_id;
    }

    static bool can_manage_catalog(const UserIdentity& user) {
        return is_administrator(user);
    }

    static bool can_set_registration_period(const UserIdentity& user) {
        return is_administrator(user);
    }

    static bool can_manage_people(const UserIdentity& user) {
        return is_administrator(user);
    }

    static bool can_open_course(const UserIdentity& user) {
        return is_teacher(user);
    }

    static bool can_review_application(const UserIdentity& user) {
        return is_administrator(user);
    }

    static bool can_submit_change_request(const UserIdentity& user,
                                          const TeachingClass& item) {
        return is_teacher(user) && item.teacher_id == user.user_id;
    }
};

}  // namespace course_selection
