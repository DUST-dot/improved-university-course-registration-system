// US-03 section
//

#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    StudentQueryService query(context);

    SeatsView view;
    check_code(query.view_available_seats(10, view).code, ResultCode::Success,
               "class 10 exists");
    check(view.teaching_class_id == 10, "view returns the class id");
    check(view.course_id == 1, "view resolves the course");
    check(view.course_code == "C001" && view.course_name == "Data Structures",
          "view carries the course code and name");
    check(view.capacity == 2, "class 10 capacity is 2");
    check(view.enrollment_count == 1, "dropped enrollment is not counted");
    check(view.remaining_seats == 1, "remaining = capacity - active");

    check_code(query.view_available_seats(11, view).code, ResultCode::Success, "class 11 exists");
    check(view.capacity == 1 && view.enrollment_count == 1 && view.remaining_seats == 0,
          "class 11 is full");

    check_code(query.view_available_seats(12, view).code, ResultCode::Success, "class 12 exists");
    check(view.capacity == 3 && view.enrollment_count == 0 && view.remaining_seats == 3,
          "class 12 is empty");

    view = SeatsView{};
    view.capacity = 99;
    check_code(query.view_available_seats(999, view).code, ResultCode::RecordNotFound,
               "unknown class returns RecordNotFound");
    check(view.capacity == 0 && view.teaching_class_id == 0,
          "failed query clears the previous capacity");

    std::vector<SeatsView> all;
    check_code(query.list_available_seats(all).code, ResultCode::Success, "list succeeds");
    check(all.size() == 3, "three teaching classes are listed");
    for (const SeatsView& item : all) {
        check(item.enrollment_count + item.remaining_seats == item.capacity,
              "count + remaining equals capacity for every row");
    }
    check(all[0].teaching_class_id == 10 && all[1].teaching_class_id == 11 &&
              all[2].teaching_class_id == 12,
          "list keeps the context order");

    context.add_enrollment(1001, 10);
    check_code(query.view_available_seats(10, view).code, ResultCode::Success, "re-query class 10");
    check(view.enrollment_count == 2 && view.remaining_seats == 0,
          "remaining seats follow the new enrollment");

    context.add_dropped_enrollment(1002, 10);
    check_code(query.view_available_seats(10, view).code, ResultCode::Success,
               "query after a dropped enrollment");
    check(view.enrollment_count == 2, "dropped record does not change the count");
    check(view.remaining_seats == 0, "remaining stays at zero");

    all.clear();
    query.list_available_seats(all);
    for (const SeatsView& item : all) {
        SeatsView single;
        query.view_available_seats(item.teaching_class_id, single);
        check(single.enrollment_count == item.enrollment_count &&
                  single.remaining_seats == item.remaining_seats,
              "list row matches the single query");
    }

    return test_support::finish();
}
