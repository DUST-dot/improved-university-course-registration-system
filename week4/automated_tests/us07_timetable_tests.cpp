// 2.17 US-07 check the class schedule
#include "test_support_week5.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    TimetableService service(context);
    std::vector<TimetableEntry> rows;

    const UserIdentity me = test_support::student();
    check_code(service.for_student(me, rows).code, ResultCode::Success, "timetable succeeds");
    check(rows.size() == 1, "one active enrollment, one meeting");
    check(rows[0].course_code == "SE101", "course code is filled in");
    check(rows[0].teacher_name == "Dr. Wang", "teacher name is filled in");
    check(rows[0].meeting.day_of_week == 1 && rows[0].meeting.start_minute == 540,
          "meeting time is filled in");

    // Add a second enrollment on a later day; the list must be sorted by day.
    context.add_enrollment(1000, 12);
    check_code(service.for_student(me, rows).code, ResultCode::Success, "timetable reloads");
    check(rows.size() == 2, "two meetings now");
    check(rows[0].meeting.day_of_week == 1 && rows[1].meeting.day_of_week == 3,
          "sorted by day of week");

    // A student with nothing enrolled gets an empty list, not an error.
    const UserIdentity other{9999, Role::Student, "s9999", "Nobody"};
    check_code(service.for_student(other, rows).code, ResultCode::RecordNotFound,
               "unknown student rejected");
    const UserIdentity empty_student{1001, Role::Student, "s1001", "Li Si"};
    context.add_dropped_enrollment(1001, 12);
    check_code(service.for_student(empty_student, rows).code, ResultCode::Success,
               "dropped enrollment yields success");
    check(rows.size() == 1, "only the active enrollment appears");

    // Teachers do not have a timetable.
    check_code(service.for_student(test_support::teacher(), rows).code, ResultCode::PermissionDenied,
               "teacher is refused");
    check(rows.empty(), "refused request clears the list");
    return test_support::finish();
}
