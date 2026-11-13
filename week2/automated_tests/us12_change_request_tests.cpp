// 2.9 US-12 调课 / 停课申请
//
// 覆盖：权限（学生不行、只能针对自己的班）、停课必须写原因、
// 撤回与重复撤回、查询的可见范围、教师维度列表、通知队列。

#include "test_support_week3.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    ChangeRequestService service(context);

    const UserIdentity teacher_a = test_support::teacher_a();   // 带班 10、12
    const UserIdentity teacher_b = test_support::teacher_b();   // 带班 11
    const UserIdentity student = test_support::student_user();
    const UserIdentity admin = test_support::admin_user();

    // ---- 权限 ----
    check_code(service.submit(student, 10, ChangeType::Reschedule, "Move to Friday").code,
               ResultCode::PermissionDenied, "student cannot submit a change request");
    check_code(service.submit(teacher_a, 11, ChangeType::Reschedule, "Move").code,
               ResultCode::PermissionDenied, "teacher cannot touch another teacher's class");
    check_code(service.submit(teacher_a, 999, ChangeType::Reschedule, "Move").code,
               ResultCode::RecordNotFound, "unknown class returns RecordNotFound");
    check(service.size() == 0, "nothing was stored by the rejected calls");

    // ---- 调课：正常提交 ----
    check_code(service.submit(teacher_a, 10, ChangeType::Reschedule, "Move to Friday 3-4").code,
               ResultCode::Success, "teacher submits a reschedule request");
    check(service.size() == 1, "the request is stored");
    check(service.pending_notices().size() == 1, "a notice is queued");
    check(service.pending_notices()[0].find("Reschedule") != std::string::npos,
          "the notice mentions the change type");

    ChangeRequest fetched;
    check_code(service.query(teacher_a, 1, fetched).code, ResultCode::Success,
               "teacher reads own request");
    check(fetched.status == ApplyStatus::Pending, "new request starts as Pending");
    check(fetched.teacher_id == 100 && fetched.teaching_class_id == 10, "ids are recorded");
    check(fetched.reason == "Move to Friday 3-4", "reason is recorded");

    // ---- 停课：有学生时必须写原因 ----
    check_code(service.submit(teacher_b, 11, ChangeType::Cancellation, "").code,
               ResultCode::InvalidInput,
               "cancelling a class with students requires a reason");
    check(service.size() == 1, "the rejected cancellation was not stored");

    check_code(service.submit(teacher_b, 11, ChangeType::Cancellation, "Teacher on leave").code,
               ResultCode::Success, "cancellation with a reason is accepted");
    check(service.size() == 2, "the cancellation is stored");

    // 没有学生的班：可以不写原因
    check_code(service.submit(teacher_a, 12, ChangeType::Cancellation, "").code,
               ResultCode::Success, "cancelling an empty class needs no reason");

    // ---- 撤回 ----
    check_code(service.withdraw(teacher_a, 1).code, ResultCode::Success,
               "teacher withdraws own request");
    check_code(service.query(teacher_a, 1, fetched).code, ResultCode::Success, "query after withdraw");
    check(fetched.status == ApplyStatus::Withdrawn, "status is Withdrawn");
    check_code(service.withdraw(teacher_a, 1).code, ResultCode::InvalidInput,
               "withdrawing twice is rejected");
    check_code(service.withdraw(teacher_b, 1).code, ResultCode::PermissionDenied,
               "another teacher cannot withdraw it");
    check_code(service.withdraw(teacher_a, 999).code, ResultCode::RecordNotFound,
               "unknown request returns RecordNotFound");

    // ---- 查询的可见范围 ----
    check_code(service.query(teacher_b, 1, fetched).code, ResultCode::PermissionDenied,
               "another teacher cannot read it");
    check(fetched.id == 0, "denied query clears the result");
    check_code(service.query(admin, 1, fetched).code, ResultCode::Success,
               "administrator can read any request");
    check_code(service.query(admin, 999, fetched).code, ResultCode::RecordNotFound,
               "unknown request returns RecordNotFound");

    // ---- 教师维度列表 ----
    std::vector<ChangeRequest> mine;
    check_code(service.list_for_teacher(100, mine).code, ResultCode::Success, "list for teacher A");
    check(mine.size() == 2, "teacher A owns two requests");
    std::vector<ChangeRequest> others;
    check_code(service.list_for_teacher(200, others).code, ResultCode::Success,
               "list for teacher B");
    check(others.size() == 1, "teacher B owns one request");

    // ---- 通知队列 ----
    check(service.pending_notices().size() == 4, "each submit and withdraw queues a notice");
    service.clear_pending_notices();
    check(service.pending_notices().empty(), "notices can be drained");

    std::cout << "  change_type_name(Cancellation) = "
              << change_type_name(ChangeType::Cancellation) << '\n';
    return test_support::finish();
}
