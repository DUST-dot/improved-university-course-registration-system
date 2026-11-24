#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "security_service.hpp"
#include "system_context.hpp"
#include "usability_service.hpp"

namespace test_support {

using course_selection::ResultCode;

inline int& checks() { static int v = 0; return v; }
inline int& failures() { static int v = 0; return v; }
inline void check(bool c, const std::string& what) {
    ++checks();
    if (!c) { ++failures(); std::cout << "FAIL: " << what << '\n'; }
}
inline void check_code(ResultCode a, ResultCode e, const std::string& what) {
    ++checks();
    if (a != e) { ++failures(); std::cout << "FAIL: " << what << '\n'; }
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
    return context;
}

inline course_selection::UserIdentity student(int id) {
    return course_selection::UserIdentity{id, course_selection::Role::Student, "s", "S"};
}
inline course_selection::UserIdentity teacher(int id) {
    return course_selection::UserIdentity{id, course_selection::Role::Teacher, "t", "T"};
}
inline course_selection::UserIdentity admin() {
    return course_selection::UserIdentity{900, course_selection::Role::Administrator, "a", "A"};
}

}  // namespace test_support
