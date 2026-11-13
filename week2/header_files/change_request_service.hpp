#pragma once

// ============================================================================
// US-12 section
//
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
