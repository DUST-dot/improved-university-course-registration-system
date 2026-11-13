// 2.2 US-02 课程筛选
//
// 覆盖：无筛选基线、学分上下限与闭区间、教师姓名关键字（含大小写与同班约束）、
// teacher_id 与关键字的冲突组合、与既有精确学分/关键词组合、非法区间。

#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

namespace {

std::string codes_of(const std::vector<CourseView>& views) {
    std::string joined;
    for (const CourseView& view : views) {
        if (!joined.empty()) joined += ",";
        joined += view.course.course_code;
    }
    return joined;
}

std::string class_ids_of(const CourseView& view) {
    std::string joined;
    for (const TeachingClassView& item : view.teaching_classes) {
        if (!joined.empty()) joined += ",";
        joined += std::to_string(item.teaching_class_id);
    }
    return joined;
}

}  // namespace

int main() {
    SystemContext context = test_support::make_context();
    context.set_now_for_tests("2026-10-05T09:00");
    context.add_enrollment(1002, 10);   // 教学班 10：容量 2、已选 1

    CourseCatalogService catalog(context);
    std::vector<CourseView> found;
    const CourseFilter no_filter;

    // ---- 基线：不带任何条件返回全部课程 ----
    check_code(catalog.search("", no_filter, found).code, ResultCode::Success,
               "search without filters succeeds");
    check(codes_of(found) == "CS101,MA101,SE101", "no filter returns all three courses");

    // ---- 学分下限 ----
    CourseFilter at_least_four;
    at_least_four.minimum_credits = 4;
    check(catalog.search("", at_least_four, found).code == ResultCode::Success,
          "lower bound search succeeds");
    check(codes_of(found) == "CS101,MA101", "credits >= 4 keeps MA101 and CS101");

    // ---- 学分上限 ----
    CourseFilter at_most_three;
    at_most_three.maximum_credits = 3;
    check(catalog.search("", at_most_three, found).code == ResultCode::Success,
          "upper bound search succeeds");
    check(codes_of(found) == "SE101", "credits <= 3 keeps only SE101");

    // ---- 闭区间：两端都能取到 ----
    CourseFilter four_to_five;
    four_to_five.minimum_credits = 4;
    four_to_five.maximum_credits = 5;
    check(catalog.search("", four_to_five, found).code == ResultCode::Success,
          "closed range search succeeds");
    check(codes_of(found) == "CS101,MA101", "range [4,5] keeps both endpoints");

    CourseFilter three_to_three;
    three_to_three.minimum_credits = 3;
    three_to_three.maximum_credits = 3;
    check(catalog.search("", three_to_three, found).code == ResultCode::Success,
          "degenerate range succeeds");
    check(codes_of(found) == "SE101", "range [3,3] equals an exact match");

    // ---- 教师姓名关键字：只保留这位老师的教学班 ----
    CourseFilter by_name;
    by_name.teacher_keyword = "wang";
    check(catalog.search("", by_name, found).code == ResultCode::Success,
          "teacher keyword search succeeds");
    check(codes_of(found) == "SE101", "keyword wang keeps only SE101");
    check(found.size() == 1 && class_ids_of(found[0]) == "10",
          "keyword wang keeps only Wang's class, not Zhao's");

    by_name.teacher_keyword = "ZHAO";
    check(catalog.search("", by_name, found).code == ResultCode::Success,
          "teacher keyword is case-insensitive");
    check(codes_of(found) == "CS101,SE101", "keyword zhao keeps CS101 and SE101");
    for (const CourseView& view : found) {
        if (view.course.course_code == "SE101") {
            check(class_ids_of(view) == "11", "SE101 lists only Zhao's class");
        }
    }

    // ---- 查不到时是"成功 + 空列表"，不是错误 ----
    CourseFilter nobody;
    nobody.teacher_keyword = "nobody";
    check_code(catalog.search("", nobody, found).code, ResultCode::Success,
               "unknown teacher is not an error");
    check(found.empty(), "unknown teacher yields an empty list");

    // ---- teacher_id 与 teacher_keyword 必须落在同一个教学班 ----
    CourseFilter conflict;
    conflict.teacher_id = 300;          // Dr. Zhao
    conflict.teacher_keyword = "wang";  // 但姓名要含 Wang
    check(catalog.search("", conflict, found).code == ResultCode::Success,
          "conflicting teacher filters are not an error");
    check(found.empty(), "teacher_id 300 + keyword wang returns nothing");

    CourseFilter consistent;
    consistent.teacher_id = 300;
    consistent.teacher_keyword = "zhao";
    check(catalog.search("", consistent, found).code == ResultCode::Success,
          "consistent teacher filters succeed");
    check(codes_of(found) == "CS101,SE101", "teacher_id 300 + keyword zhao keeps both");

    // ---- 与既有条件组合 ----
    CourseFilter combined;
    combined.minimum_credits = 5;
    check(catalog.search("program", combined, found).code == ResultCode::Success,
          "keyword with credit range succeeds");
    check(codes_of(found) == "CS101", "programming + credits>=5 keeps CS101");

    CourseFilter range_and_teacher;
    range_and_teacher.minimum_credits = 3;
    range_and_teacher.maximum_credits = 3;
    range_and_teacher.teacher_keyword = "wang";
    check(catalog.search("", range_and_teacher, found).code == ResultCode::Success,
          "range with teacher keyword succeeds");
    check(codes_of(found) == "SE101", "credits 3 + wang keeps SE101");

    CourseFilter exact;
    exact.credits = 4;
    check(catalog.search("", exact, found).code == ResultCode::Success,
          "exact credit filter still works");
    check(codes_of(found) == "MA101", "exact credits 4 keeps MA101");

    // ---- 余量现算：教学班 10 容量 2、已选 1 ----
    CourseView detail;
    check_code(catalog.detail(1, detail).code, ResultCode::Success, "course detail succeeds");
    check(detail.teaching_classes.size() == 2, "SE101 has two teaching classes");
    for (const TeachingClassView& item : detail.teaching_classes) {
        if (item.teaching_class_id == 10) {
            check(item.capacity == 2 && item.active_enrollments == 1 &&
                      item.remaining_seats == 1,
                  "remaining seats computed as capacity minus active");
        }
        if (item.teaching_class_id == 11) {
            check(item.active_enrollments == 0 && item.remaining_seats == 2,
                  "untouched class keeps full capacity");
        }
    }
    check_code(catalog.detail(999, detail).code, ResultCode::RecordNotFound,
               "missing course returns RecordNotFound");
    check(detail.course.course_code.empty(), "failed detail clears the previous result");

    // ---- 非法区间：返回 InvalidInput，而不是静默的空列表 ----
    CourseFilter reversed;
    reversed.minimum_credits = 5;
    reversed.maximum_credits = 2;
    found.clear();
    found.push_back(CourseView{});   // 放一条脏数据，验证失败时会清空
    check_code(catalog.search("", reversed, found).code, ResultCode::InvalidInput,
               "min > max is invalid");
    check(found.empty(), "invalid range clears the result list");

    CourseFilter negative;
    negative.minimum_credits = -2;
    check_code(catalog.search("", negative, found).code, ResultCode::InvalidInput,
               "negative minimum is invalid");

    CourseFilter too_large;
    too_large.maximum_credits = 31;
    check_code(catalog.search("", too_large, found).code, ResultCode::InvalidInput,
               "credit above the ceiling is invalid");

    return test_support::finish();
}
