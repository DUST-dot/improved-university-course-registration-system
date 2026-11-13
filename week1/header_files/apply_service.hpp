#pragma once

// ============================================================================
// 2.5 US-10 开课申请（移植自 se-course-system-tasks，适配统一模型）
//
// 有状态流转的功能：用枚举 + 迁移表，规则写在一处，不在业务里堆 if-else。
// 改动点只有两类：类型名对齐（course_id / UserIdentity 字段）、命名空间收进
// course_selection；业务判断与返回码原样保留。
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

enum class ApplyStatus { Draft, Pending, Approved, Rejected, Withdrawn };

const char* apply_status_name(ApplyStatus status);

// 合法迁移表：改规则只改这一个函数
bool can_transition(ApplyStatus from, ApplyStatus to);

struct ApplyRecord {
    int apply_id{0};
    int course_id{0};
    int teacher_id{0};
    std::string semester;
    ApplyStatus status{ApplyStatus::Draft};
    std::string review_comment;   // 审批意见，全程可查
};

class ApplyService {
public:
    explicit ApplyService(SystemContext& context) : context_(context) {}

    // 暂存：只创建草稿，不改任何已有单据
    OperationResult create_draft(const UserIdentity& user,
                                 int course_id,
                                 const std::string& semester,
                                 ApplyRecord& created);

    // 提交：草稿 -> 待审批；非法状态或重复申请都返回失败且不改数据
    OperationResult submit(int apply_id);

    // 撤回：待审批 -> 已撤回
    OperationResult withdraw(int apply_id);

    // 审批：待审批 -> 通过 / 驳回，可写审批意见
    OperationResult review(int apply_id, bool approve, const std::string& comment);

    OperationResult query(int apply_id, ApplyRecord& result) const;
    OperationResult list_for_teacher(int teacher_id, std::vector<ApplyRecord>& result) const;

    std::size_t size() const { return records_.size(); }

private:
    ApplyRecord* find(int apply_id);
    const ApplyRecord* find(int apply_id) const;

    SystemContext& context_;
    std::vector<ApplyRecord> records_;
    int next_id_{1};
};

}  // namespace course_selection
