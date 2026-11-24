#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "release_check.hpp"
#include "system_context.hpp"

namespace test_support {

inline int& checks() { static int v = 0; return v; }
inline int& failures() { static int v = 0; return v; }
inline void check(bool c, const std::string& what) {
    ++checks();
    if (!c) { ++failures(); std::cout << "FAIL: " << what << '\n'; }
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
    TeachingClass c10{}; c10.teaching_class_id = 10; c10.course_id = 1; c10.teacher_id = 100;
    c10.capacity = 2;
    context.add_teaching_class(c10);
    context.add_enrollment(1000, 10);
    return context;
}

}  // namespace test_support
