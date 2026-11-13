// 2.20 US-20 data reliability
#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    IntegrityService service(context);
    std::vector<std::string> problems;

    check_code(service.check(problems).code, ResultCode::Success, "clean data passes");
    check(problems.empty(), "no problems reported");

    // A duplicate active record is caught.
    context.add_enrollment(1000, 10);
    check_code(service.check(problems).code, ResultCode::InvalidInput, "duplicate detected");
    check(problems.size() == 1, "one problem reported");
    check(problems[0].find("Duplicate") != std::string::npos, "problem names the duplicate");

    // Over capacity is caught as well.
    SystemContext small = test_support::make_context();
    small.add_enrollment(1000, 10);
    small.add_enrollment(1001, 10);
    IntegrityService small_service(small);
    small_service.check(problems);
    bool over = false;
    for (const std::string& p : problems) if (p.find("Over capacity") != std::string::npos) over = true;
    check(over, "over capacity is reported");
    check(problems.size() >= 2, "all problems are returned, not just the first");
    return test_support::finish();
}
