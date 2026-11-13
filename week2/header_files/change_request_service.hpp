#pragma once

// ============================================================================
// 2.9 US-12 调课 / 停课申请（移植自 se-course-system-tasks，适配统一模型）
//
// 复用 2.5 的 ApplyStatus 状态机：申请提交后进入 Pending，可撤回、可审批。
// 额外一条硬约束：已有学生选课的班不允许静默停课，必须写明原因——
// 这条只能在提交时校验，审批时补就晚了。
// ============================================================================

#include <string>
#include <vector>

#include "apply_service.hpp"
#include "system_context.hpp"

namespace course_selection {

enum class ChangeType { Reschedule, Cancellation };

const char* change_type_name(ChangeType type);

struct ChangeRequest {
    int id{0};
    int teaching_class_id{0};
    int teacher_id{0};
    ChangeType type{ChangeType::Reschedule};
    ApplyStatus status{ApplyStatus::Draft};
    std::string reason;
    std::string review_comment;
};

class ChangeRequestService {
public:
    explicit ChangeRequestService(SystemContext& context) : context_(context) {}

    OperationResult submit(const UserIdentity& user,
                           int teaching_class_id,
                           ChangeType type,
                           const std::string& reason);

    OperationResult withdraw(const UserIdentity& user, int request_id);
    OperationResult query(const UserIdentity& user, int request_id, ChangeRequest& result) const;
    OperationResult list_for_teacher(int teacher_id, std::vector<ChangeRequest>& result) const;

    // 待发送通知：先落队列，由演示层统一打印，不阻塞提交
    const std::vector<std::string>& pending_notices() const { return pending_notices_; }
    void clear_pending_notices() { pending_notices_.clear(); }

    std::size_t size() const { return requests_.size(); }

private:
    ChangeRequest* find(int request_id);
    const ChangeRequest* find(int request_id) const;

    SystemContext& context_;
    std::vector<ChangeRequest> requests_;
    std::vector<std::string> pending_notices_;
    int next_id_{1};
};

}  // namespace course_selection
