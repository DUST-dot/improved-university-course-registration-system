#pragma once

// ============================================================================
// 2.6 US-03 实时剩余名额（移植自 se-course-system-tasks，适配统一模型）
//
// 只读视图：容量 / 有效人数 / 剩余名额，三个数一次给出。
// 人数与余量都从 SystemContext 现算，与教师端、管理员端同一口径。
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct SeatsView {
    int teaching_class_id{0};
    int course_id{0};
    std::string course_code;
    std::string course_name;
    int capacity{0};
    int enrollment_count{0};
    int remaining_seats{0};
};

class StudentQueryService {
public:
    explicit StudentQueryService(const SystemContext& context) : context_(context) {}

    // 单个教学班的余量；教学班不存在返回 RecordNotFound，而不是 0 个名额
    OperationResult view_available_seats(int teaching_class_id, SeatsView& result) const;

    // 课程列表：每个教学班一行，余量口径与上者完全相同
    OperationResult list_available_seats(std::vector<SeatsView>& result) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
