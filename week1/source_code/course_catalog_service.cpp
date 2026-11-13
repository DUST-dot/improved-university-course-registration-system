#include "course_catalog_service.hpp"

#include <algorithm>
#include <cctype>

namespace course_selection {
namespace {

std::string to_lower(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

bool contains_text(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    return haystack.find(needle) != std::string::npos;
}

bool by_teaching_class_id(const TeachingClassView& left, const TeachingClassView& right) {
    return left.teaching_class_id < right.teaching_class_id;
}

}  // namespace

CourseCatalogService::CourseCatalogService(const SystemContext& context)
    : context_(context) {}

bool CourseCatalogService::filter_is_valid(const CourseFilter& filter) const {
    if (filter.minimum_credits < -1 || filter.maximum_credits < -1) return false;
    if (filter.minimum_credits > 30 || filter.maximum_credits > 30) return false;
    if (filter.minimum_credits >= 0 && filter.maximum_credits >= 0 &&
        filter.minimum_credits > filter.maximum_credits) {
        return false;
    }
    return true;
}

bool CourseCatalogService::class_passes_teacher_filter(const TeachingClass& item,
                                                      const CourseFilter& filter) const {
    if (filter.teacher_id >= 0 && item.teacher_id != filter.teacher_id) return false;
    if (!filter.teacher_keyword.empty()) {
        const Teacher* teacher = context_.find_teacher(item.teacher_id);
        if (teacher == nullptr) return false;
        if (!contains_text(to_lower(teacher->name), to_lower(filter.teacher_keyword))) {
            return false;
        }
    }
    return true;
}

bool CourseCatalogService::course_matches(const Course& course,
                                          const std::string& lowered_keyword,
                                          const CourseFilter& filter) const {
    if (!filter.course_code.empty() &&
        to_lower(course.course_code) != to_lower(filter.course_code)) {
        return false;
    }
    if (filter.credits >= 0 && course.credits != filter.credits) return false;
    if (filter.minimum_credits >= 0 && course.credits < filter.minimum_credits) return false;
    if (filter.maximum_credits >= 0 && course.credits > filter.maximum_credits) return false;

    const bool teacher_filter_active =
        filter.teacher_id >= 0 || !filter.teacher_keyword.empty();
    bool has_candidate_class = false;
    bool teacher_name_hit = false;
    for (const TeachingClass& item : context_.teaching_classes()) {
        if (item.course_id != course.course_id) continue;
        if (!class_passes_teacher_filter(item, filter)) continue;
        has_candidate_class = true;
        const Teacher* teacher = context_.find_teacher(item.teacher_id);
        if (teacher != nullptr &&
            contains_text(to_lower(teacher->name), lowered_keyword)) {
            teacher_name_hit = true;
        }
    }
    if (teacher_filter_active && !has_candidate_class) return false;

    const bool course_level_match =
        contains_text(to_lower(course.course_code), lowered_keyword) ||
        contains_text(to_lower(course.course_name), lowered_keyword);
    return course_level_match || teacher_name_hit;
}

CourseView CourseCatalogService::build_view(const Course& course,
                                            const CourseFilter& filter) const {
    CourseView view;
    view.course = course;
    for (const TeachingClass& item : context_.teaching_classes()) {
        if (item.course_id != course.course_id) continue;
        if (!class_passes_teacher_filter(item, filter)) continue;

        TeachingClassView class_view{};
        class_view.teaching_class_id = item.teaching_class_id;
        class_view.teacher_id = item.teacher_id;
        const Teacher* teacher = context_.find_teacher(item.teacher_id);
        class_view.teacher_name = teacher == nullptr ? std::string() : teacher->name;
        class_view.capacity = item.capacity;
        class_view.active_enrollments = context_.enrollment_count(item.teaching_class_id);
        class_view.remaining_seats = context_.remaining_seats(item.teaching_class_id);
        class_view.meetings = item.meetings;
        view.teaching_classes.push_back(class_view);
    }
    std::sort(view.teaching_classes.begin(), view.teaching_classes.end(),
              by_teaching_class_id);
    return view;
}

OperationResult CourseCatalogService::search(const std::string& keyword,
                                             const CourseFilter& filter,
                                             std::vector<CourseView>& result) const {
    result.clear();
    if (!filter_is_valid(filter)) {
        return {ResultCode::InvalidInput,
                "Invalid filter: credit range must satisfy 0 <= min <= max"};
    }

    const std::string lowered_keyword = to_lower(keyword);
    for (const Course& course : context_.courses()) {
        if (!course_matches(course, lowered_keyword, filter)) continue;
        result.push_back(build_view(course, filter));
    }
    std::sort(result.begin(), result.end(),
              [](const CourseView& left, const CourseView& right) {
                  return left.course.course_code < right.course.course_code;
              });
    return {ResultCode::Success, "Search completed"};
}

OperationResult CourseCatalogService::detail(int course_id, CourseView& result) const {
    result = CourseView{};
    Course course{};
    if (!context_.find_course(course_id, course)) {
        return {ResultCode::RecordNotFound, "Course not found"};
    }
    result = build_view(course, CourseFilter{});
    return {ResultCode::Success, "Course found"};
}

}  // namespace course_selection
