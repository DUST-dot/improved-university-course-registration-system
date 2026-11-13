#pragma once

// Week 1 test support, written against the unified model.

#include <iostream>
#include <string>
#include <vector>

#include "administration_service.hpp"
#include "authentication_service.hpp"
#include "course_catalog_service.hpp"
#include "system_context.hpp"
#include "teacher_service.hpp"

namespace test_support {

using course_selection::ResultCode;

inline int& checks() { static int v = 0; return v; }
inline int& failures() { static int v = 0; return v; }

inline void check(bool condition, const std::string& what) {
    ++checks();
    if (!condition) { ++failures(); std::cout << "FAIL: " << what << '\n'; }
}

inline void check_code(ResultCode actual, ResultCode expected, const std::string& what) {
    ++checks();
    if (actual != expected) {
        ++failures();
        std::cout << "FAIL: " << what << " (got "
                  << course_selection::result_code_name(actual) << ", expected "
                  << course_selection::result_code_name(expected) << ")\n";
    }
}

inline int finish() {
    std::cout << "PASS: " << checks() << " checks, " << failures() << " failure(s)\n";
    return failures() == 0 ? 0 : 1;
}

inline course_selection::SystemContext make_context() {
    using namespace course_selection;

    SystemContext context;
    context.add_course(Course{1, "SE101", "Software Engineering", 3.0, {}});
    context.add_course(Course{2, "MA101", "Mathematics", 4.0, {}});
    context.add_course(Course{3, "CS101", "Programming", 5.0, {}});

    context.add_student(Student{1001, "S2023001", "Alice Chen", "Computer Science", true});
    context.add_student(Student{1002, "S2023002", "Bob Liu", "Computer Science", true});
    context.add_student(Student{1003, "S2023003", "Carol Wu", "Mathematics", false});

    context.add_teacher(Teacher{100, "T2015007", "Dr. Wang", "Software Engineering", true});
    context.add_teacher(Teacher{200, "T2016011", "Dr. Li", "Mathematics", true});
    context.add_teacher(Teacher{300, "T2018003", "Dr. Zhao", "Computer Science", true});

    context.add_administrator(Administrator{900, "A2020001", "Admin Sun", true});

    TeachingClass c10{};
    c10.teaching_class_id = 10;
    c10.course_id = 1;
    c10.teacher_id = 100;
    c10.capacity = 2;
    c10.time_slot = "MON-1-2";
    c10.room = "A101";
    c10.semester = "2026-2027-1";
    c10.meetings = {ClassMeeting{1, 540, 600, WeekType::All, "A101"}};
    context.add_teaching_class(c10);

    TeachingClass c11{};
    c11.teaching_class_id = 11;
    c11.course_id = 1;
    c11.teacher_id = 300;
    c11.capacity = 2;
    c11.time_slot = "TUE-1-2";
    c11.room = "A102";
    c11.meetings = {ClassMeeting{2, 540, 600, WeekType::All, "A102"}};
    context.add_teaching_class(c11);

    TeachingClass c20{};
    c20.teaching_class_id = 20;
    c20.course_id = 2;
    c20.teacher_id = 200;
    c20.capacity = 2;
    c20.time_slot = "MON-1-2";
    c20.room = "B201";
    c20.meetings = {ClassMeeting{1, 570, 630, WeekType::All, "B201"}};
    context.add_teaching_class(c20);

    TeachingClass c30{};
    c30.teaching_class_id = 30;
    c30.course_id = 3;
    c30.teacher_id = 300;
    c30.capacity = 2;
    c30.time_slot = "WED-6-7";
    c30.room = "C301";
    c30.meetings = {ClassMeeting{3, 840, 900, WeekType::All, "C301"}};
    context.add_teaching_class(c30);

    RegistrationRule rule;
    // The round starts closed: the administrator opens it by setting a period,
    // which is exactly what the credit-cap lock depends on.
    rule.registration_open = false;
    rule.maximum_credits = 25;
    context.apply_rule_change(rule);
    context.set_now_for_tests("2026-10-05T09:00");
    return context;
}

inline std::vector<course_selection::AccountCredential> credentials() {
    using course_selection::AuthenticationService;
    using course_selection::Role;
    return {AuthenticationService::credential_for(1001, "S2023001", Role::Student, "Abcd1234"),
            AuthenticationService::credential_for(1002, "S2023002", Role::Student, "Bob12345"),
            AuthenticationService::credential_for(1003, "S2023003", Role::Student, "Carol123"),
            AuthenticationService::credential_for(100, "T2015007", Role::Teacher, "Teach1234"),
            AuthenticationService::credential_for(900, "A2020001", Role::Administrator,
                                                  "Admin1234")};
}

inline course_selection::UserIdentity student_identity() {
    return course_selection::UserIdentity{1001, course_selection::Role::Student, "S2023001",
                                          "Alice Chen"};
}

inline course_selection::UserIdentity teacher_identity(int teacher_id = 100) {
    return course_selection::UserIdentity{teacher_id, course_selection::Role::Teacher,
                                          "T2015007", "Dr. Wang"};
}

inline course_selection::UserIdentity admin_identity() {
    return course_selection::UserIdentity{900, course_selection::Role::Administrator,
                                          "A2020001", "Admin Sun"};
}

}  // namespace test_support
