#pragma once

// The single source of truth: catalogue, accounts and enrollment records exist here
// only. Services take a reference and never keep a copy, so the teacher view and the
// seat view cannot disagree.
//
// Three rules:
//   1. Remaining seats = capacity - active count, computed on every read.
//   2. A dropped enrollment is kept, but never counted.
//   3. Time is always read through now(); tests pin it with set_now_for_tests.

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

    // ---- catalogue: read-only views, callers cannot swap the containers ----
    const std::vector<Course>& courses() const { return courses_; }
    const std::vector<TeachingClass>& teaching_classes() const { return teaching_classes_; }
    const std::vector<Student>& students() const { return students_; }
    const std::vector<Teacher>& teachers() const { return teachers_; }
    const std::vector<Administrator>& administrators() const { return administrators_; }

    // Seeding helpers for tests and demos; business code must not call these.
    void add_course(const Course& course) { courses_.push_back(course); }
    void add_teaching_class(const TeachingClass& item) { teaching_classes_.push_back(item); }
    void add_student(const Student& student) { students_.push_back(student); }
    void add_teacher(const Teacher& teacher) { teachers_.push_back(teacher); }
    void add_administrator(const Administrator& admin) { administrators_.push_back(admin); }

    // ---- lookups ----
    bool find_course(int course_id, Course& course) const;
    bool find_teaching_class(int teaching_class_id, TeachingClass& item) const;
    const Student* find_student(int student_id) const;
    const Teacher* find_teacher(int teacher_id) const;
    const Administrator* find_administrator(int administrator_id) const;
    const Student* find_student_by_number(const std::string& number) const;
    const Teacher* find_teacher_by_number(const std::string& number) const;

    // ---- enrollment records ----
    const std::vector<Enrollment>& all_enrollments() const { return enrollments_; }
    void add_enrollment(int student_id, int teaching_class_id);
    void add_dropped_enrollment(int student_id, int teaching_class_id);

    // Flips one Active record to Dropped. Returns false when there is nothing to
    // drop, so the caller reports ActiveEnrollmentNotFound instead of success.
    bool drop_active_enrollment(int student_id, int teaching_class_id);

    int enrollment_count(int teaching_class_id) const;
    int remaining_seats(int teaching_class_id) const;
    std::vector<Enrollment> active_enrollments(int student_id) const;

    // ---- rules ----
    const RegistrationRule& rule() const { return rule_; }
    void apply_rule_change(const RegistrationRule& rule) { rule_ = rule; }
    void apply_rule(const RegistrationRule& rule) { apply_rule_change(rule); }

    // ---- rounds (US-16) ----
    const std::vector<RegistrationRound>& registration_rounds() const { return rounds_; }
    void add_round(const RegistrationRound& round) { rounds_.push_back(round); }

    // ---- time ----
    std::string now() const;
    void set_now_for_tests(const std::string& iso_minute) { fixed_now_ = iso_minute; }

    // Once a round has been seen open it stays open for this purpose, which is what
    // locks the credit cap (US-13): reopen-after-raise must not be possible.
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
