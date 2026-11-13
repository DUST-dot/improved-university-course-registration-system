// 2.16 US-18 enrollment distribution chart
#include "test_support_week5.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    AnalyticsService service(context);
    std::vector<std::pair<std::string, int>> rows;

    check_code(service.distribution("course", rows).code, ResultCode::Success, "by course works");
    check(rows.size() == 2, "two courses appear");
    check(rows[0].first == "MA101" && rows[0].second == 1, "MA101 counted once");
    check(rows[1].first == "SE101" && rows[1].second == 1, "dropped record not counted");

    check_code(service.distribution("college", rows).code, ResultCode::Success, "by college works");
    check(rows.size() == 2, "two departments appear");
    check(rows[0].second == 1 && rows[1].second == 1, "one each");

    // An additional enrollment changes the ordering.
    context.add_enrollment(1000, 12);   // SE101 again
    service.distribution("course", rows);
    check(rows[0].first == "SE101" && rows[0].second == 2, "SE101 is now first with two");

    // Unknown dimension must fail, not return an empty chart.
    check_code(service.distribution("teacher", rows).code, ResultCode::InvalidInput,
               "unknown dimension rejected");
    check(rows.empty(), "result is cleared on failure");
    return test_support::finish();
}
