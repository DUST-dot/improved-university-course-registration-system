#pragma once

// ============================================================================
// SystemContext：统一后的唯一数据源
//
// 合并了两套写法：
//   - week1 的私有成员 + 访问器（防止外部直接改内部容器）
//   - 2.5–2.9 需要公开读取的目录（通过 courses() / teaching_classes() 返回值引用）
//
// 三条不变的约定：
//   1. 余量 = capacity - 有效选课人数，每次现算，不存计数器。
//   2. 退课记录保留为 Dropped，计入历史但不计入人数。
//   3. 时间统一从 now() 取，测试用 set_now_for_tests 固定。
// ============================================================================

#include <string>
#include <vector>

#include "domain.hpp"

namespace course_selection {

class SystemContext {
public:
    SystemContext() = default;

    SystemContext(std::vector<Student> students,
                  std::vector<Teacher> teachers,
                  std::vector<Administrator> administrators,
                  std::vector<Course> courses,
                  std::vector<TeachingClass> teaching_classes,
                  RegistrationRule rule)
        : students_(std::move(students)),
          teachers_(std::move(teachers)),
          administrators_(std::move(administrators)),
          courses_(std::move(courses)),
          teaching_classes_(std::move(teaching_classes)),
          rule_(rule) {}

    // ---- 目录：返回 const 引用，调用方不能替换容器 ----
    const std::vector<Course>& courses() const { return courses_; }
    const std::vector<TeachingClass>& teaching_classes() const { return teaching_classes_; }
    const std::vector<Student>& students() const { return students_; }
    const std::vector<Teacher>& teachers() const { return teachers_; }
    const std::vector<Administrator>& administrators() const { return administrators_; }

    // 演示/测试铺数据用；业务代码不应调用。
    void add_course(const Course& course) { courses_.push_back(course); }
    void add_teaching_class(const TeachingClass& item) { teaching_classes_.push_back(item); }
    void add_student(const Student& student) { students_.push_back(student); }
    void add_teacher(const Teacher& teacher) { teachers_.push_back(teacher); }

    // ---- 查找 ----
    bool find_course(int course_id, Course& course) const;
    bool find_teaching_class(int teaching_class_id, TeachingClass& item) const;
    const Student* find_student(int student_id) const;
    const Teacher* find_teacher(int teacher_id) const;
    const Student* find_student_by_number(const std::string& number) const;
    const Teacher* find_teacher_by_number(const std::string& number) const;

    // ---- 选课记录 ----
    const std::vector<Enrollment>& all_enrollments() const { return enrollments_; }
    void add_enrollment(int student_id, int teaching_class_id);
    void add_dropped_enrollment(int student_id, int teaching_class_id);

    // Flips one Active record to Dropped. Returns false when there is nothing to
    // drop, so the caller can report ActiveEnrollmentNotFound instead of success.
    bool drop_active_enrollment(int student_id, int teaching_class_id);

    int enrollment_count(int teaching_class_id) const;
    int remaining_seats(int teaching_class_id) const;
    std::vector<Enrollment> active_enrollments(int student_id) const;

    // ---- 选课规则 ----
    const RegistrationRule& rule() const { return rule_; }
    void apply_rule_change(const RegistrationRule& rule) { rule_ = rule; }
    // week1 的旧名字，保留以兼容两边代码风格
    void apply_rule(const RegistrationRule& rule) { apply_rule_change(rule); }

    // ---- 轮次（US-16）----
    const std::vector<RegistrationRound>& registration_rounds() const { return rounds_; }
    void add_round(const RegistrationRound& round) { rounds_.push_back(round); }

    // ---- 时间 ----
    std::string now() const;
    void set_now_for_tests(const std::string& iso_minute) { fixed_now_ = iso_minute; }

    // 本轮是否已经开放过：一旦观察到开放就保持 true，用于锁住学分上限（US-13）
    bool observe_registration_round() {
        if (rule_.registration_open) round_opened_ = true;
        return round_opened_;
    }
    bool round_opened() const { return round_opened_; }

private:
    std::vector<Student> students_;
    std::vector<Teacher> teachers_;
    std::vector<Administrator> administrators_;
    std::vector<Course> courses_;
    std::vector<TeachingClass> teaching_classes_;
    std::vector<Enrollment> enrollments_;
    std::vector<RegistrationRound> rounds_;
    RegistrationRule rule_;
    std::string fixed_now_;
    bool round_opened_{false};
};

}  // namespace course_selection
