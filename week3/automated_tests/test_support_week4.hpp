#pragma once

// Week 4 test support: assertion counters plus one fixed data set that exercises
// week types, prerequisites and a full class.

#include <iostream>
#include <string>
#include <vector>

#include "apply_service.hpp"
#include "conflict_service.hpp"
#include "enrollment_service.hpp"
#include "import_service.hpp"
#include "review_service.hpp"
#include "system_context.hpp"

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
        std::cout << "FAIL: " << what << " (got " << course_selection::result_code_name(actual)
                  << ", expected " << course_selection::result_code_name(expected) << ")\n";
    }
}

inline int finish() {
    std::cout << "PASS: " << checks() << " checks, " << failures() << " failure(s)\n";
    return failures() == 0 ? 0 : 1;
}

// Courses:
//   1 SE101 3 credits, no prerequisite
//   2 MA101 4 credits, requires course 1
//   3 CS101 5 credits, no prerequisite
// Classes:
//   10 -> course 1, teacher 100, capacity 2, MON 09:00-10:00 all weeks
//   11 -> course 2, teacher 200, capacity 1, MON 09:30-10:30 all weeks (clashes with 10)
//   12 -> course 3, teacher 100, capacity 3, TUE 09:00-10:00 odd weeks
//   13 -> course 3, teacher 200, capacity 3, TUE 09:00-10:00 even weeks
inline course_selection::SystemContext make_context() {
    using namespace course_selection;
    SystemContext context;
    context.add_course(Course{1, "SE101", "Software Engineering", 3.0, {}});
    context.add_course(Course{2, "MA101", "Mathematics", 4.0, {1}});
    context.add_course(Course{3, "CS101", "Programming", 5.0, {}});
    context.add_student(Student{1000, "20230001", "Zhang San"});
    context.add_student(Student{1001, "20230002", "Li Si"});

    TeachingClass c10{}; c10.teaching_class_id = 10; c10.course_id = 1; c10.teacher_id = 100;
    c10.capacity = 2; c10.meetings = {ClassMeeting{1, 540, 600, WeekType::All, "A101"}};
    context.add_teaching_class(c10);

    TeachingClass c11{}; c11.teaching_class_id = 11; c11.course_id = 2; c11.teacher_id = 200;
    c11.capacity = 1; c11.meetings = {ClassMeeting{1, 570, 630, WeekType::All, "B203"}};
    context.add_teaching_class(c11);

    TeachingClass c12{}; c12.teaching_class_id = 12; c12.course_id = 3; c12.teacher_id = 100;
    c12.capacity = 3; c12.meetings = {ClassMeeting{2, 540, 600, WeekType::Odd, "A204"}};
    context.add_teaching_class(c12);

    TeachingClass c13{}; c13.teaching_class_id = 13; c13.course_id = 3; c13.teacher_id = 200;
    c13.capacity = 3; c13.meetings = {ClassMeeting{2, 540, 600, WeekType::Even, "A205"}};
    context.add_teaching_class(c13);

    context.set_now_for_tests("2026-10-12T09:00");
    // Week 4 exercises enrollment, so the window starts open.
    RegistrationRule rule;
    rule.registration_open = true;
    context.apply_rule_change(rule);
    return context;
}

inline course_selection::UserIdentity teacher_a() {
    return course_selection::UserIdentity{100, course_selection::Role::Teacher, "t100", "Teacher A"};
}
inline course_selection::UserIdentity admin_user() {
    return course_selection::UserIdentity{900, course_selection::Role::Administrator, "admin", "Admin"};
}

}  // namespace test_support
