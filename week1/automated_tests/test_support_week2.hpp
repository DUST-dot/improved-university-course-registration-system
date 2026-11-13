#pragma once

// ============================================================================
//
//
// ============================================================================

#include <iostream>
#include <string>
#include <vector>

#include "apply_service.hpp"
#include "system_context.hpp"

namespace test_support {

using course_selection::ResultCode;

inline int& checks() {
    static int value = 0;
    return value;
}

inline int& failures() {
    static int value = 0;
    return value;
}

inline void check(bool condition, const std::string& what) {
    ++checks();
    if (!condition) {
        ++failures();
        std::cout << "FAIL: " << what << '\n';
    }
}

inline void check_code(ResultCode actual, ResultCode expected, const std::string& what) {
    ++checks();
    if (actual != expected) {
        ++failures();
        std::cout << "FAIL: " << what << " (got " << static_cast<int>(actual)
                  << " = " << course_selection::result_code_name(actual) << ", expected "
                  << static_cast<int>(expected) << " = "
                  << course_selection::result_code_name(expected) << ")\n";
    }
}

inline int finish() {
    std::cout << "PASS: " << checks() << " checks, " << failures() << " failure(s)\n";
    return failures() == 0 ? 0 : 1;
}

inline course_selection::SystemContext make_context() {
    using course_selection::Course;
    using course_selection::EnrollmentStatus;
    using course_selection::Student;
    using course_selection::SystemContext;
    using course_selection::TeachingClass;

    SystemContext context;
    context.add_course(Course{1, "C001", "Data Structures", 4.0});
    context.add_course(Course{2, "C002", "Operating Systems", 3.5});

    context.add_student(Student{1000, "20230001", "Zhang San"});
    context.add_student(Student{1001, "20230002", "Li Si"});
    context.add_student(Student{1002, "20230003", "Wang Wu"});

    context.add_teaching_class(TeachingClass{10, 1, 100, 2, "MON-1-2", "A101", "2026-2027-1"});
    context.add_teaching_class(TeachingClass{11, 2, 200, 1, "TUE-3-4", "B203", "2026-2027-1"});
    context.add_teaching_class(TeachingClass{12, 1, 100, 3, "WED-1-2", "A204", "2026-2027-1"});

    context.add_enrollment(1000, 10);
    context.add_dropped_enrollment(1001, 10);
    context.add_enrollment(1002, 11);

    context.set_now_for_tests("2026-10-12T09:00");
    return context;
}

inline course_selection::UserIdentity teacher_a() {
    return course_selection::UserIdentity{100, course_selection::Role::Teacher, "t100",
                                          "Teacher A"};
}
inline course_selection::UserIdentity teacher_b() {
    return course_selection::UserIdentity{200, course_selection::Role::Teacher, "t200",
                                          "Teacher B"};
}
inline course_selection::UserIdentity student_user() {
    return course_selection::UserIdentity{1000, course_selection::Role::Student, "s1000",
                                          "Zhang San"};
}
inline course_selection::UserIdentity admin_user() {
    return course_selection::UserIdentity{900, course_selection::Role::Administrator, "admin",
                                          "Admin"};
}

}  // namespace test_support
