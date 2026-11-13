#include "seats_service.hpp"

namespace course_selection {

OperationResult StudentQueryService::view_available_seats(int teaching_class_id,
                                                          SeatsView& result) const {
    result = SeatsView{};   // 进来先清空，失败时不留下上一次的容量与余量

    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::RecordNotFound, "Teaching class not found"};
    }

    Course course{};
    context_.find_course(item.course_id, course);

    result.teaching_class_id = item.teaching_class_id;
    result.course_id = item.course_id;
    result.course_code = course.course_code;
    result.course_name = course.course_name;
    result.capacity = item.capacity;
    result.enrollment_count = context_.enrollment_count(item.teaching_class_id);
    result.remaining_seats = context_.remaining_seats(item.teaching_class_id);
    return {ResultCode::Success, "Seats available"};
}

OperationResult StudentQueryService::list_available_seats(std::vector<SeatsView>& result) const {
    result.clear();
    for (const TeachingClass& item : context_.teaching_classes()) {
        SeatsView view{};
        const OperationResult one = view_available_seats(item.teaching_class_id, view);
        if (!ok(one)) continue;
        result.push_back(view);
    }
    return {ResultCode::Success, "Seats list ready"};
}

}  // namespace course_selection
