#pragma once
// Week 5 test support: counters and one fixed data set.
#include <iostream>
#include <string>
#include <vector>

#include "monitor_service.hpp"
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
        std::cout << "FAIL: " << what << " (got "
                  << course_selection::result_code_name(actual) << ")\n";
    }
}

inline int finish() {
    std::cout << "PASS: " << checks() << " checks, " << failures() << " failure(s)\n";
    return failures() == 0 ? 0 : 1;
}

// Class 10: capacity 2, one active + one dropped student.
// Class 11: capacity 1, one active student, therefore full.
// Class 12: capacity 3, empty.
inline course_selection::SystemContext make_context() {
    using namespace course_selection;
    SystemContext context;
    context.add_course(Course{1, "SE101", "Software Engineering", 3.0, {}});
    context.add_course(Course{2, "MA101", "Mathematics", 4.0, {}});
    context.add_student(Student{1000, "20230001", "Zhang San"});
    context.add_student(Student{1001, "20230002", "Li Si"});
    context.add_teacher(Teacher{100, "T100", "Dr. Wang", "Software Engineering"});
    context.add_teacher(Teacher{200, "T200", "Dr. Li", "Mathematics"});

    TeachingClass c10{}; c10.teaching_class_id = 10; c10.course_id = 1; c10.teacher_id = 100;
    c10.capacity = 2; c10.meetings = {ClassMeeting{1, 540, 600, WeekType::All, "A101"}};
    context.add_teaching_class(c10);

    TeachingClass c11{}; c11.teaching_class_id = 11; c11.course_id = 2; c11.teacher_id = 200;
    c11.capacity = 1; c11.meetings = {ClassMeeting{2, 540, 600, WeekType::All, "B203"}};
    context.add_teaching_class(c11);

    TeachingClass c12{}; c12.teaching_class_id = 12; c12.course_id = 1; c12.teacher_id = 100;
    c12.capacity = 3; c12.meetings = {ClassMeeting{3, 840, 900, WeekType::All, "A204"}};
    context.add_teaching_class(c12);

    context.add_enrollment(1000, 10);
    context.add_dropped_enrollment(1001, 10);
    context.add_enrollment(1001, 11);
    context.set_now_for_tests("2026-10-12T09:00");
    return context;
}

inline course_selection::UserIdentity student() {
    return course_selection::UserIdentity{1000, course_selection::Role::Student, "s1000", "Zhang San"};
}
inline course_selection::UserIdentity teacher() {
    return course_selection::UserIdentity{100, course_selection::Role::Teacher, "t100", "Dr. Wang"};
}

}  // namespace test_support
