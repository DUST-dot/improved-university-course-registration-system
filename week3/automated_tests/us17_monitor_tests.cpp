// 2.14 US-17 real-time monitoring
#include "test_support_week5.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    MonitorService service(context);
    MonitorSnapshot snapshot;

    check_code(service.monitor(snapshot).code, ResultCode::Success, "monitor succeeds");
    check(snapshot.teaching_classes == 3, "three teaching classes counted");
    check(snapshot.full_classes == 1, "class 11 is the only full one");
    check(snapshot.active_enrollments == 2, "dropped enrollment is not counted");
    // 1 of 3 full is below the 80% threshold, so no alarm.
    check(snapshot.alarms.size() == 1, "one entry in the alarm list");
    check(snapshot.alarms[0] == "No alarm", "no alarm below the threshold");

    // Fill class 10 as well: 2 of 3 full is still below 80%? 2/3 = 0.67, so no alarm.
    context.add_enrollment(1000, 10);
    service.monitor(snapshot);
    check(snapshot.full_classes == 2, "class 10 is full now");
    check(snapshot.alarms[0] == "No alarm", "0.67 is still below the threshold");

    // Fill class 12 (capacity 3): 3 of 3 classes full is above the threshold.
    context.add_enrollment(1001, 12);
    context.add_enrollment(1000, 12);
    context.add_enrollment(1001, 12);
    service.monitor(snapshot);
    check(snapshot.full_classes == 3, "all classes are full");
    check(snapshot.alarms[0].find("80%") != std::string::npos, "alarm mentions the threshold");

    // An empty context must not divide by zero.
    SystemContext empty;
    MonitorService empty_service(empty);
    check_code(empty_service.monitor(snapshot).code, ResultCode::Success, "empty context works");
    check(snapshot.teaching_classes == 0 && snapshot.alarms[0] == "No alarm",
          "empty context reports no alarm");
    return test_support::finish();
}
