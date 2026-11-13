// US-13 section
//

#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    context.set_now_for_tests("2026-10-05T09:00");

    AuthenticationService auth(context, test_support::credentials());
    UserIdentity identity;
    std::string token;

    check_code(auth.login("A2020001", "Admin1234", identity, token).code, ResultCode::Success,
               "administrator login succeeds");
    check(identity.user_id == 900, "admin identity keeps administrator_id");
    check(identity.role == Role::Administrator, "admin identity has Administrator role");
    check(!token.empty(), "admin login issues a token");

    check_code(auth.login("A2020001", "Nope1234", identity, token).code,
               ResultCode::AuthenticationFailed, "admin wrong password fails");
    check(identity.user_id == 0 && token.empty(), "failed admin login clears identity");

    const UserIdentity admin = test_support::admin_identity();
    const UserIdentity wang = test_support::teacher_identity(100);
    const UserIdentity student = test_support::student_identity();

    check_code(AuthenticationService::require_role(admin, Role::Administrator).code,
               ResultCode::Success, "admin passes admin entry");
    check_code(AuthenticationService::require_role(wang, Role::Administrator).code,
               ResultCode::PermissionDenied, "teacher rejected by admin entry");
    check_code(AuthenticationService::require_role(student, Role::Administrator).code,
               ResultCode::PermissionDenied, "student rejected by admin entry");

    AdministrationService administration(context);

    check(context.rule().maximum_credits == 25, "initial credit cap is 25");
    check_code(administration.set_maximum_credits(admin, 30).code, ResultCode::Success,
               "admin can raise the credit cap before the round opens");
    check(context.rule().maximum_credits == 30, "credit cap is updated");

    check_code(administration.set_maximum_credits(admin, 0).code, ResultCode::InvalidInput,
               "zero is not a valid cap");
    check_code(administration.set_maximum_credits(admin, 61).code, ResultCode::InvalidInput,
               "cap above 60 is rejected");
    check_code(administration.set_maximum_credits(admin, -5).code, ResultCode::InvalidInput,
               "negative cap is rejected");
    check(context.rule().maximum_credits == 30, "invalid inputs left the cap untouched");

    check_code(administration.set_maximum_credits(wang, 40).code, ResultCode::PermissionDenied,
               "teacher cannot change the credit cap");
    check_code(administration.set_maximum_credits(student, 40).code, ResultCode::PermissionDenied,
               "student cannot change the credit cap");
    check(context.rule().maximum_credits == 30, "unauthorised change left the cap untouched");

    check_code(administration.set_registration_period(admin, "2026-10-12T09:00",
                                                      "2026-10-16T17:00").code,
               ResultCode::Success, "admin sets a valid registration period");
    check(context.rule().opens_at == "2026-10-12T09:00", "opening time is stored");
    check(context.rule().closes_at == "2026-10-16T17:00", "closing time is stored");
    check(context.rule().registration_open, "setting a period opens the round");

    check_code(administration.set_registration_period(admin, "2026-10-12", "2026-10-16T17:00").code,
               ResultCode::InvalidInput, "date without time is rejected");
    check_code(administration.set_registration_period(admin, "abc", "2026-10-16T17:00").code,
               ResultCode::InvalidInput, "non-date text is rejected");
    check_code(administration.set_registration_period(admin, "2026-10-16T17:00",
                                                      "2026-10-12T09:00").code,
               ResultCode::InvalidInput, "opening later than closing is rejected");
    check_code(administration.set_registration_period(admin, "2026-10-12T09:00",
                                                      "2026-10-12T09:00").code,
               ResultCode::InvalidInput, "identical opening and closing is rejected");
    check(context.rule().opens_at == "2026-10-12T09:00", "invalid periods left the window alone");

    check_code(administration.set_registration_period(wang, "2026-10-12T09:00",
                                                      "2026-10-16T17:00").code,
               ResultCode::PermissionDenied, "teacher cannot set the period");
    check_code(administration.set_registration_period(student, "2026-10-12T09:00",
                                                      "2026-10-16T17:00").code,
               ResultCode::PermissionDenied, "student cannot set the period");

    check(context.observe_registration_round(), "round is observed as opened");
    check_code(administration.set_maximum_credits(admin, 40).code, ResultCode::PermissionDenied,
               "credit cap is locked once the round has opened");
    check(context.rule().maximum_credits == 30, "locked cap is unchanged");

    const RegistrationRoundStatus before_close = administration.current_round();
    check(before_close.opened, "current round reports opened");
    check(before_close.opens_at == "2026-10-12T09:00", "current round reports the window");

    check_code(administration.close_registration(admin).code, ResultCode::Success,
               "admin closes registration");
    check(!context.rule().registration_open, "registration is closed");
    check(context.rule().opens_at.empty() && context.rule().closes_at.empty(),
          "closing clears the window so a refresh cannot reopen it");
    check_code(administration.close_registration(wang).code, ResultCode::PermissionDenied,
               "teacher cannot close registration");

    return test_support::finish();
}
