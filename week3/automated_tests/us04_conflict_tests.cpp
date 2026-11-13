// 2.10 US-04 time conflict reminder

#include "test_support_week4.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    // Interval and week rules first: everything else depends on them.
    check(intervals_overlap(540, 600, 570, 630), "09:00-10:00 overlaps 09:30-10:30");
    check(!intervals_overlap(540, 600, 600, 660), "adjacent slots do not overlap");
    check(weeks_compatible(WeekType::All, WeekType::Odd), "all-week meets odd-week");
    check(weeks_compatible(WeekType::Odd, WeekType::Odd), "odd meets odd");
    check(!weeks_compatible(WeekType::Odd, WeekType::Even), "odd never meets even");

    SystemContext context = test_support::make_context();
    ConflictService service(context);
    std::vector<Conflict> conflicts;

    // Nothing enrolled yet: the course is clear.
    check_code(service.detect(1000, 1, conflicts).code, ResultCode::Success, "detect succeeds");
    check(conflicts.empty(), "no enrollment means no conflict");

    // Enrolled in 10 (SE101, MON 09:00-10:00); MA101 sits at 09:30-10:30 on the same day.
    // Enrolling in course 1 also satisfies the prerequisite of course 2, so the
    // only problem left is the time clash.
    context.add_enrollment(1000, 10);
    check_code(service.detect(1000, 2, conflicts).code, ResultCode::Success, "detect succeeds");
    check(conflicts.size() == 1, "one time clash, prerequisite already satisfied");
    check(conflicts[0].kind == Conflict::Kind::TimeClash, "the clash is reported");
    check(conflicts[0].other_course_id == 1, "clash names the existing course");
    check(conflicts[0].message.find("MON 09:00-10:00") != std::string::npos,
          "message contains the existing slot");
    check(conflicts[0].message.find("09:30-10:30") != std::string::npos,
          "message contains the target slot");

    // Student 1001 holds nothing, so the prerequisite of course 2 is missing.
    check_code(service.detect(1001, 2, conflicts).code, ResultCode::Success, "detect succeeds");
    check(conflicts.size() == 1, "missing prerequisite is reported");
    check(conflicts[0].kind == Conflict::Kind::MissingPrerequisite, "kind is MissingPrerequisite");
    check(conflicts[0].other_course_id == 1, "prerequisite id is reported");

    // A different day is clear.
    check_code(service.detect(1000, 3, conflicts).code, ResultCode::Success, "detect succeeds");
    check(conflicts.empty(), "different day has no clash");

    // Unknown course.
    check_code(service.detect(1000, 999, conflicts).code, ResultCode::RecordNotFound,
               "unknown course returns RecordNotFound");
    check(conflicts.empty(), "failed detect clears the list");

    // Submit wrapper maps the first conflict to a result code.
    std::string detail;
    check_code(service.check_before_submit(1001, 2, detail).code, ResultCode::InvalidInput,
               "missing prerequisite maps to InvalidInput");
    check(!detail.empty(), "detail is filled in");

    std::string clear_detail;
    check_code(service.check_before_submit(1001, 3, clear_detail).code, ResultCode::Success,
               "clear course passes the check");
    check(clear_detail.empty(), "no detail when there is no conflict");

    return test_support::finish();
}
