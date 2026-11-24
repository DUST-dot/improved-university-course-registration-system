// 2.23 US-23 maintainability and 2.24 US-24 release and buffer
#include "test_support.hpp"

using namespace course_selection;
using test_support::check;

int main() {
    SystemContext context = test_support::make_context();
    ReleaseCheck check_tool(context);
    const ReleaseReport report = check_tool.run();

    check(report.failures == 0, "a healthy project passes the release check");
    check(report.checks >= 5, "at least five invariants are checked");
    check(report.details.empty(), "no details when everything passes");

    // A broken catalogue must fail the check, not slip through.
    SystemContext broken;
    broken.add_course(Course{1, "SE101", "Software Engineering", 3.0, {}});
    TeachingClass orphan{}; orphan.teaching_class_id = 20; orphan.course_id = 999;
    orphan.teacher_id = 100; orphan.capacity = 0;
    broken.add_teaching_class(orphan);
    ReleaseCheck broken_tool(broken);
    const ReleaseReport bad = broken_tool.run();
    check(bad.failures > 0, "a broken catalogue fails the check");
    check(!bad.details.empty(), "the failure is explained");

    // Seat arithmetic holds on the healthy context.
    for (const TeachingClass& item : context.teaching_classes()) {
        check(context.enrollment_count(item.teaching_class_id) +
                      context.remaining_seats(item.teaching_class_id) ==
                  item.capacity,
              "count plus remaining equals capacity");
    }
    return test_support::finish();
}
