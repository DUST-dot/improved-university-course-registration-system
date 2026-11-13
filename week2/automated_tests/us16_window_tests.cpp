// US-16 section
//

#include "test_support_week3.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    RegistrationWindowService window(context);

    const UserIdentity admin = test_support::admin_user();
    const UserIdentity teacher = test_support::teacher_a();
    const UserIdentity student = test_support::student_user();

    WindowState state = WindowState::Open;
    check_code(window.window_state_at("2026-10-12T09:00", state).code, ResultCode::Success,
               "window state can be asked");
    check(state == WindowState::Closed, "initial window is closed");

    check_code(window.set_manual_switch(admin, true).code, ResultCode::Success,
               "administrator flips the manual switch");
    window.window_state_at("2026-10-12T09:00", state);
    check(state == WindowState::Open, "manual switch opens the window");
    check_code(window.set_manual_switch(admin, false).code, ResultCode::Success,
               "administrator flips it back");
    window.window_state_at("2026-10-12T09:00", state);
    check(state == WindowState::Closed, "manual switch closes the window");

    check_code(window.set_registration_period(teacher, "2026-10-12T09:00", "2026-10-16T17:00").code,
               ResultCode::PermissionDenied, "teacher cannot set the period");
    check_code(window.set_registration_period(student, "2026-10-12T09:00", "2026-10-16T17:00").code,
               ResultCode::PermissionDenied, "student cannot set the period");
    check_code(window.set_manual_switch(teacher, true).code, ResultCode::PermissionDenied,
               "teacher cannot use the manual switch");

    check_code(window.set_registration_period(admin, "2026-10-12", "2026-10-16T17:00").code,
               ResultCode::InvalidInput, "missing time part is rejected");
    check_code(window.set_registration_period(admin, "2026-13-01T09:00", "2026-13-02T09:00").code,
               ResultCode::InvalidInput, "month 13 is rejected");
    check_code(window.set_registration_period(admin, "2026-02-30T09:00", "2026-03-02T09:00").code,
               ResultCode::InvalidInput, "February 30 is rejected");
    check_code(window.set_registration_period(admin, "2026-10-16T17:00", "2026-10-12T09:00").code,
               ResultCode::InvalidInput, "opening later than closing is rejected");
    check_code(window.set_registration_period(admin, "2026-10-12T09:00", "2026-10-12T09:00").code,
               ResultCode::InvalidInput, "identical bounds are rejected");
    check(window.rounds().empty(), "no round was recorded by the rejected calls");

    check_code(window.set_registration_period(admin, "2026-10-12T09:00", "2026-10-16T17:00").code,
               ResultCode::Success, "administrator sets a valid period");
    check(window.rounds().size() == 1, "the round is recorded");

    window.window_state_at("2026-10-12T08:59", state);
    check(state == WindowState::NotStarted, "before opening: NotStarted");
    window.window_state_at("2026-10-12T09:00", state);
    check(state == WindowState::Open, "at opening: Open");
    window.window_state_at("2026-10-15T12:00", state);
    check(state == WindowState::Open, "inside the window: Open");
    window.window_state_at("2026-10-16T17:00", state);
    check(state == WindowState::Closed, "at closing: Closed (half-open interval)");
    window.window_state_at("2026-10-20T09:00", state);
    check(state == WindowState::Closed, "after closing: Closed");

    check_code(window.set_registration_period(admin, "2026-10-14T09:00", "2026-10-18T17:00").code,
               ResultCode::InvalidInput, "an overlapping round is rejected");
    check(window.rounds().size() == 1, "overlapping call did not record a round");

    check_code(window.set_registration_period(admin, "2026-10-12T09:00", "2026-10-16T17:00").code,
               ResultCode::Success, "resubmitting the same round is allowed");
    check(window.rounds().size() == 1, "the same round is not duplicated");

    check_code(window.set_registration_period(admin, "2026-10-20T09:00", "2026-10-24T17:00").code,
               ResultCode::Success, "a non-overlapping second round is allowed");
    check(window.rounds().size() == 2, "the second round is recorded");

    // ---- current_rule ----
    RegistrationRule rule;
    check_code(window.current_rule(rule).code, ResultCode::Success, "current rule is readable");
    check(rule.opens_at == "2026-10-20T09:00" && rule.closes_at == "2026-10-24T17:00",
          "current rule holds the latest period");
    check(rule.registration_open, "setting a period also opens the round");

    std::cout << "  window_state_name(Open) = " << window_state_name(WindowState::Open) << '\n';
    return test_support::finish();
}
