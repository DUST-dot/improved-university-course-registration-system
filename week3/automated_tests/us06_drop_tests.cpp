// 2.13 US-06 drop class, plus the feedback path used by US-05

#include "test_support_week4.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    EnrollmentService service(context);
    EnrollOutcome outcome;

    // ---- enroll ----
    check_code(service.enroll(1000, 10, outcome).code, ResultCode::Success, "enroll succeeds");
    check(outcome.active_enrollments == 1 && outcome.remaining_seats == 1,
          "counters reflect the new enrollment");
    check(outcome.message == "Enrolled", "message is ready for the UI");

    // Duplicate enrollment in the same class.
    check_code(service.enroll(1000, 10, outcome).code, ResultCode::AlreadyEnrolled,
               "duplicate enrollment is refused");
    check(context.enrollment_count(10) == 1, "duplicate did not change the count");

    // Capacity: class 11 holds one student, so the second one is refused.
    check_code(service.enroll(1001, 11, outcome).code, ResultCode::Success,
               "class 11 takes its first student");
    check_code(service.enroll(1000, 11, outcome).code, ResultCode::TeachingClassFull,
               "class 11 is full");
    check(outcome.remaining_seats == 0, "full class reports zero remaining");

    // Unknown class.
    check_code(service.enroll(1000, 999, outcome).code, ResultCode::TeachingClassNotFound,
               "unknown class is refused");

    // Credit cap: student 1000 now holds 3 credits (SE101). CS101 is 5 credits.
    RegistrationRule rule = context.rule();
    rule.maximum_credits = 6;
    context.apply_rule_change(rule);
    check_code(service.enroll(1000, 12, outcome).code, ResultCode::CreditLimitExceeded,
               "credit cap is enforced");
    rule.maximum_credits = 25;
    context.apply_rule_change(rule);

    // Closed window.
    rule.registration_open = false;
    context.apply_rule_change(rule);
    check_code(service.enroll(1000, 12, outcome).code, ResultCode::RegistrationClosed,
               "closed window refuses enrollment");
    rule.registration_open = true;
    context.apply_rule_change(rule);

    check(service.active_credits(1000) == 3.0, "active credits add up");

    // ---- drop ----
    check_code(service.drop(1000, 10, outcome).code, ResultCode::Success, "drop succeeds");
    check(outcome.active_enrollments == 0 && outcome.remaining_seats == 2,
          "seats are released immediately");
    check(context.enrollment_count(10) == 0, "count drops to zero");

    // The record is kept as Dropped, not deleted.
    bool kept_as_dropped = false;
    for (const Enrollment& record : context.all_enrollments()) {
        if (record.student_id == 1000 && record.teaching_class_id == 10 &&
            record.status == EnrollmentStatus::Dropped) {
            kept_as_dropped = true;
        }
    }
    check(kept_as_dropped, "dropped record is kept for history");
    check(service.active_credits(1000) == 0.0, "credits are released after the drop");

    // Dropping twice must not look successful.
    check_code(service.drop(1000, 10, outcome).code, ResultCode::ActiveEnrollmentNotFound,
               "second drop is refused");

    // Dropping a class the student never took.
    check_code(service.drop(1001, 12, outcome).code, ResultCode::ActiveEnrollmentNotFound,
               "nothing to drop");

    // Enrollment works again after the drop.
    check_code(service.enroll(1000, 10, outcome).code, ResultCode::Success,
               "student can enroll again after dropping");
    check(context.enrollment_count(10) == 1, "count is one again");

    return test_support::finish();
}
