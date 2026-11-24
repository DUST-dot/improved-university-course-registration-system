// 2.22 US-22 usability
#include "test_support.hpp"

using namespace course_selection;
using test_support::check;

int main() {
    check(UsabilityService::validate_login("S2023001", "Abcd1234").empty(),
          "valid login passes");
    check(!UsabilityService::validate_login("S2", "Abcd1234").empty(), "short account flagged");
    check(!UsabilityService::validate_login("S2023 001", "Abcd1234").empty(),
          "illegal character flagged");
    check(!UsabilityService::validate_login("S2023001", "").empty(), "empty password flagged");

    check(UsabilityService::validate_enroll_form("10").empty(), "numeric class id passes");
    check(!UsabilityService::validate_enroll_form("").empty(), "empty class id flagged");
    check(!UsabilityService::validate_enroll_form("abc").empty(), "non numeric class id flagged");

    check(UsabilityService::hint_for(ResultCode::TeachingClassFull).find("seats") !=
              std::string::npos, "full class hint mentions seats");
    check(!UsabilityService::hint_for(ResultCode::TimeConflict).empty(),
          "every code has a hint");
    return test_support::finish();
}
