// 2.19 US-19 performance reliability (moved here: the story belongs to week 6)
#include "test_support.hpp"

using namespace course_selection;
using test_support::check;

int main() {
    SystemContext context = test_support::make_context();
    PerformanceService service(context);
    service.rebuild_index();

    check(service.count_by_scan(10) == 1, "scan counts one active record");
    check(service.count_by_index(10) == 1, "index agrees with the scan");
    check(service.count_by_scan(99) == 0, "unknown class counts zero");
    check(service.count_by_index(99) == 0, "index returns zero for an unknown class");

    service.reset_counters();
    service.count_by_scan(10);
    check(service.records_scanned_by_scan() == context.all_enrollments().size(),
          "a scan touches every record");

    service.reset_counters();
    for (int i = 0; i < 50; ++i) service.count_by_index(10);
    check(service.records_scanned_by_scan() == 0, "the index path scans nothing");
    check(service.index_calls() == 50, "every lookup is counted");

    const int before = service.count_by_index(10);
    context.add_enrollment(1001, 10);
    check(service.count_by_index(10) == before, "index is stale until rebuilt");
    service.rebuild_index();
    check(service.count_by_index(10) == before + 1, "rebuild sees the new record");
    return test_support::finish();
}
