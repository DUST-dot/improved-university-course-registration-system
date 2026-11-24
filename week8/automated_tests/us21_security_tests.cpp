// 2.21 US-21 security
#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    SecurityService service(context);

    // Horizontal: a student may only act on their own record.
    check_code(service.assert_can_act_on_student(test_support::student(1000), 1000).code,
               ResultCode::Success, "own record allowed");
    check_code(service.assert_can_act_on_student(test_support::student(1000), 1001).code,
               ResultCode::PermissionDenied, "another student is refused");
    check_code(service.assert_can_act_on_student(test_support::admin(), 1001).code,
               ResultCode::Success, "administrator may act on anyone");

    // Vertical: a teacher may only touch their own class.
    check_code(service.assert_can_act_on_class(test_support::teacher(100), 10).code,
               ResultCode::Success, "own class allowed");
    check_code(service.assert_can_act_on_class(test_support::teacher(200), 10).code,
               ResultCode::PermissionDenied, "another teacher refused");
    check_code(service.assert_can_act_on_class(test_support::student(1000), 10).code,
               ResultCode::PermissionDenied, "student refused");
    check_code(service.assert_can_act_on_class(test_support::admin(), 10).code,
               ResultCode::Success, "administrator allowed");
    check_code(service.assert_can_act_on_class(test_support::admin(), 99).code,
               ResultCode::RecordNotFound, "unknown class reported");

    // Audit trail is append only.
    service.record(test_support::admin(), "class:update", "10");
    service.record(test_support::teacher(100), "request:submit", "10");
    check(service.audit().size() == 2, "two audit entries");
    check(service.audit()[0].action == "class:update", "action is recorded");
    return test_support::finish();
}
