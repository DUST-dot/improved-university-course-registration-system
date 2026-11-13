// 2.8 US-11 教师端：班级概览与名单
//
// 覆盖：人数与余量口径、越权、名单只含 Active、按学号排序、
// 以及合并后保留的 week1 接口 assigned_teaching_classes。

#include "test_support_week3.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    TeacherService service(context);

    const UserIdentity teacher_a = test_support::teacher_a();     // 工号 100，带班 10、12
    const UserIdentity teacher_b = test_support::teacher_b();     // 工号 200，带班 11
    const UserIdentity admin = test_support::admin_user();
    const UserIdentity student = test_support::student_user();

    // ---- 班级概览 ----
    TeachingClassSummary summary;
    check_code(service.class_summary(teacher_a, 10, summary).code, ResultCode::Success,
               "teacher reads own class 10");
    check(summary.teaching_class_id == 10 && summary.course_id == 1, "ids are filled in");
    check(summary.course_code == "C001" && summary.course_name == "Data Structures",
          "course info is filled in");
    check(summary.capacity == 2, "capacity is 2");
    check(summary.active_enrollments == 1, "dropped enrollment is not counted");
    check(summary.remaining_seats == 1, "remaining = capacity - active");

    check_code(service.class_summary(teacher_a, 11, summary).code, ResultCode::PermissionDenied,
               "teacher cannot read another teacher's class");
    check(summary.teaching_class_id == 0 && summary.capacity == 0,
          "denied summary is cleared");

    check_code(service.class_summary(teacher_a, 999, summary).code, ResultCode::RecordNotFound,
               "missing class returns RecordNotFound");

    check_code(service.class_summary(admin, 11, summary).code, ResultCode::Success,
               "administrator can read any class");
    check(summary.capacity == 1 && summary.active_enrollments == 1 &&
              summary.remaining_seats == 0,
          "class 11 is full");

    check_code(service.class_summary(student, 10, summary).code, ResultCode::PermissionDenied,
               "student has no teacher entry");

    // ---- 名单 ----
    ClassRoster roster;
    check_code(service.class_roster(teacher_a, 10, roster).code, ResultCode::Success,
               "teacher reads own roster");
    check(roster.teaching_class_id == 10, "roster carries the class id");
    check(roster.students.size() == 1, "dropped student is not listed");
    check(!roster.students.empty() && roster.students[0].student_number == "20230001",
          "remaining student is 20230001");
    check(!roster.students.empty() && roster.students[0].student_id == 1000,
          "roster entry keeps the student id");

    // 再加一名学生，验证排序
    context.add_enrollment(1001, 10);
    check_code(service.class_roster(teacher_a, 10, roster).code, ResultCode::Success,
               "roster reloads");
    check(roster.students.size() == 2, "two active students now");
    check(roster.students[0].student_number == "20230001" &&
              roster.students[1].student_number == "20230002",
          "roster is sorted by student number");

    check_code(service.class_roster(teacher_b, 10, roster).code, ResultCode::PermissionDenied,
               "another teacher cannot read this roster");
    check(roster.students.empty(), "denied roster is cleared");

    // ---- 合并保留的 week1 接口：列出本人教学班 ----
    std::vector<TeachingClassSummary> own;
    check_code(service.assigned_teaching_classes(teacher_a, own).code, ResultCode::Success,
               "teacher lists own classes");
    check(own.size() == 2 && own[0].teaching_class_id == 10 && own[1].teaching_class_id == 12,
          "teacher A owns 10 and 12, sorted");

    check_code(service.assigned_teaching_classes(teacher_b, own).code, ResultCode::Success,
               "teacher B lists own classes");
    check(own.size() == 1 && own[0].teaching_class_id == 11, "teacher B owns only 11");

    check_code(service.assigned_teaching_classes(admin, own).code, ResultCode::Success,
               "administrator lists all classes");
    check(own.size() == 3, "administrator sees all three");

    own.clear();
    check_code(service.assigned_teaching_classes(student, own).code, ResultCode::PermissionDenied,
               "student cannot list teaching classes");
    check(own.empty(), "student gets an empty list");

    return test_support::finish();
}
