// 2.18 US-08 course recommendation
#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    RecommendationService service(context);
    std::vector<Recommendation> rows;

    check_code(service.recommend(1001, 10, rows).code, ResultCode::Success,
               "recommendation succeeds");
    // Student 1001 holds class 11 (full) and has a dropped record in class 10.
    check(rows.size() == 2, "class 10 and class 12 are candidates");
    for (const Recommendation& row : rows) {
        check(row.teaching_class_id != 11, "full class is never recommended");
    }

    // Already enrolled classes are excluded.
    check_code(service.recommend(1000, 10, rows).code, ResultCode::Success, "second student works");
    for (const Recommendation& row : rows) {
        check(row.teaching_class_id != 10, "enrolled class is excluded");
    }
    check(!rows.empty(), "there is something left to recommend");
    check(!rows[0].reason.empty(), "every recommendation explains itself");

    // The limit is honoured and the order is descending by score.
    std::vector<Recommendation> limited;
    service.recommend(1001, 1, limited);
    check(limited.size() == 1, "limit is applied");
    service.recommend(1001, 10, rows);
    for (std::size_t i = 1; i < rows.size(); ++i) {
        check(rows[i - 1].score >= rows[i].score, "sorted by score");
    }

    // Unknown student.
    check_code(service.recommend(9999, 10, rows).code, ResultCode::RecordNotFound,
               "unknown student rejected");
    check(rows.empty(), "result cleared on failure");
    return test_support::finish();
}
