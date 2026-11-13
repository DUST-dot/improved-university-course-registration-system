#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "integrity_service.hpp"
#include "performance_service.hpp"
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

inline course_selection::SystemContext make_context() {
    using namespace course_selection;
    SystemContext context;
    context.add_course(Course{1, "SE101", "Software Engineering", 3.0, {}});
    context.add_student(Student{1000, "20230001", "Zhang San"});
    context.add_student(Student{1001, "20230002", "Li Si"});
    TeachingClass c10{}; c10.teaching_class_id = 10; c10.course_id = 1; c10.teacher_id = 100;
    c10.capacity = 2;
    context.add_teaching_class(c10);
    context.add_enrollment(1000, 10);
    context.set_now_for_tests("2026-10-12T09:00");
    return context;
}

}  // namespace test_support
