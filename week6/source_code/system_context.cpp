#include "system_context.hpp"

#include "time_util.hpp"

namespace course_selection {

bool SystemContext::find_course(int course_id, Course& course) const {
    for (const Course& item : courses_) {
        if (item.course_id == course_id) {
            course = item;
            return true;
        }
    }
    return false;
}

bool SystemContext::find_teaching_class(int teaching_class_id, TeachingClass& item) const {
    for (const TeachingClass& candidate : teaching_classes_) {
        if (candidate.teaching_class_id == teaching_class_id) {
            item = candidate;
            return true;
        }
    }
    return false;
}

const Student* SystemContext::find_student(int student_id) const {
    for (const Student& item : students_) {
        if (item.student_id == student_id) return &item;
    }
    return nullptr;
}

const Teacher* SystemContext::find_teacher(int teacher_id) const {
    for (const Teacher& item : teachers_) {
        if (item.teacher_id == teacher_id) return &item;
    }
    return nullptr;
}

const Administrator* SystemContext::find_administrator(int administrator_id) const {
    for (const Administrator& item : administrators_) {
        if (item.administrator_id == administrator_id) return &item;
    }
    return nullptr;
}

const Student* SystemContext::find_student_by_number(const std::string& number) const {
    for (const Student& item : students_) {
        if (item.student_number == number) return &item;
    }
    return nullptr;
}

const Teacher* SystemContext::find_teacher_by_number(const std::string& number) const {
    for (const Teacher& item : teachers_) {
        if (item.teacher_number == number) return &item;
    }
    return nullptr;
}

void SystemContext::add_enrollment(int student_id, int teaching_class_id) {
    enrollments_.push_back({student_id, teaching_class_id, EnrollmentStatus::Active});
}

void SystemContext::add_dropped_enrollment(int student_id, int teaching_class_id) {
    enrollments_.push_back({student_id, teaching_class_id, EnrollmentStatus::Dropped});
}

bool SystemContext::drop_active_enrollment(int student_id, int teaching_class_id) {
    for (Enrollment& record : enrollments_) {
        if (record.student_id == student_id &&
            record.teaching_class_id == teaching_class_id &&
            record.status == EnrollmentStatus::Active) {
            record.status = EnrollmentStatus::Dropped;   // keep the record, change the status
            return true;
        }
    }
    return false;
}

int SystemContext::enrollment_count(int teaching_class_id) const {
    int count = 0;
    for (const Enrollment& record : enrollments_) {
        if (record.teaching_class_id == teaching_class_id &&
            record.status == EnrollmentStatus::Active) {
            ++count;
        }
    }
    return count;
}

int SystemContext::remaining_seats(int teaching_class_id) const {
    TeachingClass item{};
    if (!find_teaching_class(teaching_class_id, item)) return 0;
    // The only seat calculation; every view calls it, so the numbers always agree.
    return item.capacity - enrollment_count(teaching_class_id);
}

std::vector<Enrollment> SystemContext::active_enrollments(int student_id) const {
    std::vector<Enrollment> result;
    for (const Enrollment& record : enrollments_) {
        if (record.student_id == student_id && record.status == EnrollmentStatus::Active) {
            result.push_back(record);
        }
    }
    return result;
}

std::string SystemContext::now() const {
    return fixed_now_.empty() ? current_timestamp() : fixed_now_;
}

}  // namespace course_selection
