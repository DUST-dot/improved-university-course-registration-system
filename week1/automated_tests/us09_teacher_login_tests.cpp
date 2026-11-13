// 2.3 US-09 教师登录
//
// 覆盖：教师登录成功、角色守卫（学生进不去教师端、教师进不去教务端）、
// 教学班列表只含本人、单个教学班越权读不到、名单只含 Active 且按学号排序。

#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

namespace {

std::vector<int> ids_of(const std::vector<TeachingClassSummary>& items) {
    std::vector<int> ids;
    for (const TeachingClassSummary& item : items) ids.push_back(item.teaching_class_id);
    return ids;
}

}  // namespace

int main() {
    SystemContext context = test_support::make_context();
    context.set_now_for_tests("2026-10-05T09:00");
    // 教学班 10 有 1 条有效选课，另加 1 条已退课记录（应不出现在名单里）
    context.add_enrollment(1002, 10);
    context.add_dropped_enrollment(1003, 10);

    AuthenticationService auth(context, test_support::credentials());
    UserIdentity identity;
    std::string token;

    // ---- 登录成功 ----
    check_code(auth.login("T2015007", "Teach1234", identity, token).code, ResultCode::Success,
               "teacher login succeeds");
    check(identity.user_id == 100, "teacher identity keeps teacher_id");
    check(identity.account_number == "T2015007", "teacher identity keeps employee number");
    check(identity.role == Role::Teacher, "teacher identity has Teacher role");
    check(!token.empty(), "teacher login issues a token");

    check_code(auth.login("T2015007", "Wrong1234", identity, token).code,
               ResultCode::AuthenticationFailed, "teacher wrong password fails");
    check(identity.user_id == 0 && token.empty(), "failed teacher login clears identity");

    // ---- 角色守卫：三个方向的越权都要拦住 ----
    const UserIdentity student = test_support::student_identity();
    const UserIdentity wang = test_support::teacher_identity(100);
    const UserIdentity admin = test_support::admin_identity();

    check_code(AuthenticationService::require_role(student, Role::Teacher).code,
               ResultCode::PermissionDenied, "student rejected by teacher entry");
    check_code(AuthenticationService::require_role(wang, Role::Teacher).code,
               ResultCode::Success, "teacher passes teacher entry");
    check_code(AuthenticationService::require_role(wang, Role::Administrator).code,
               ResultCode::PermissionDenied, "teacher rejected by admin entry");
    check_code(AuthenticationService::require_role(wang, Role::Student).code,
               ResultCode::PermissionDenied, "teacher rejected by student entry");

    // ---- 教学班列表：只含本人 ----
    TeacherService teacher_service(context);
    std::vector<TeachingClassSummary> classes;

    check_code(teacher_service.assigned_teaching_classes(wang, classes).code, ResultCode::Success,
               "teacher lists own classes");
    check(classes.size() == 1, "Wang teaches exactly one class");
    check(!classes.empty() && classes[0].teaching_class_id == 10, "Wang's class is id 10");
    check(!classes.empty() && classes[0].course_code == "SE101", "class 10 belongs to SE101");
    check(!classes.empty() && classes[0].teacher_name == "Dr. Wang", "teacher name is filled in");
    check(!classes.empty() && classes[0].capacity == 2, "capacity is 2");
    check(!classes.empty() && classes[0].active_enrollments == 1,
          "only active enrollments are counted");
    check(!classes.empty() && classes[0].remaining_seats == 1,
          "remaining seats = capacity - active");

    const UserIdentity zhao = test_support::teacher_identity(300);
    check_code(teacher_service.assigned_teaching_classes(zhao, classes).code, ResultCode::Success,
               "Zhao lists own classes");
    check(ids_of(classes) == std::vector<int>({11, 30}), "Zhao owns classes 11 and 30");
    check(ids_of(classes) == std::vector<int>({11, 30}), "list is sorted by teaching_class_id");

    // 学生走教师入口：守卫直接拒绝，不返回任何班级
    classes.clear();
    check_code(teacher_service.assigned_teaching_classes(student, classes).code,
               ResultCode::PermissionDenied, "student cannot list teaching classes");
    check(classes.empty(), "student gets an empty list, not other people's classes");

    // 管理员可以列全部
    check_code(teacher_service.assigned_teaching_classes(admin, classes).code, ResultCode::Success,
               "administrator lists all classes");
    check(classes.size() == 4, "administrator sees all four classes");

    // ---- 单个教学班：越权读不到 ----
    TeachingClassSummary summary;
    check_code(teacher_service.class_summary(wang, 10, summary).code, ResultCode::Success,
               "teacher reads own class summary");
    check(summary.teaching_class_id == 10 && summary.remaining_seats == 1,
          "own class summary is correct");

    summary = TeachingClassSummary{};
    check_code(teacher_service.class_summary(wang, 30, summary).code, ResultCode::PermissionDenied,
               "teacher cannot read another teacher's class");
    check(summary.teaching_class_id == 0, "denied summary is cleared");

    check_code(teacher_service.class_summary(wang, 999, summary).code, ResultCode::RecordNotFound,
               "missing class returns RecordNotFound");

    check_code(teacher_service.class_summary(admin, 30, summary).code, ResultCode::Success,
               "administrator can read any class");

    // ---- 名单：只含 Active，按学号排序 ----
    ClassRoster roster;
    check_code(teacher_service.class_roster(wang, 10, roster).code, ResultCode::Success,
               "teacher reads own roster");
    check(roster.students.size() == 1, "dropped enrollment is not listed");
    check(!roster.students.empty() && roster.students[0].student_number == "S2023002",
          "remaining student is S2023002");
    check(roster.capacity == 2 && roster.active_enrollments == 1 && roster.remaining_seats == 1,
          "roster counts match the summary");

    // 再加一个学生，验证按学号排序
    context.add_enrollment(1001, 10);
    check_code(teacher_service.class_roster(wang, 10, roster).code, ResultCode::Success,
               "roster reloads after a new enrollment");
    check(roster.students.size() == 2, "two active students now");
    check(roster.students[0].student_number == "S2023001" &&
              roster.students[1].student_number == "S2023002",
          "roster is sorted by student number");
    check(roster.active_enrollments == 2 && roster.remaining_seats == 0,
          "remaining seats follow the new count");

    check_code(teacher_service.class_roster(zhao, 10, roster).code, ResultCode::PermissionDenied,
               "another teacher cannot read this roster");
    check(roster.students.empty(), "denied roster is cleared");

    return test_support::finish();
}
