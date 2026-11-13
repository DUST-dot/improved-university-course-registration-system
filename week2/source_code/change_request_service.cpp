#include "change_request_service.hpp"

#include <string>

#include "permission_rules.hpp"

namespace course_selection {

const char* change_type_name(ChangeType type) {
    switch (type) {
        case ChangeType::Reschedule:   return "Reschedule";
        case ChangeType::Cancellation: return "Cancellation";
    }
    return "Unknown";
}

ChangeRequest* ChangeRequestService::find(int request_id) {
    for (ChangeRequest& request : requests_) {
        if (request.id == request_id) return &request;
    }
    return nullptr;
}

const ChangeRequest* ChangeRequestService::find(int request_id) const {
    for (const ChangeRequest& request : requests_) {
        if (request.id == request_id) return &request;
    }
    return nullptr;
}

OperationResult ChangeRequestService::submit(const UserIdentity& user,
                                             int teaching_class_id,
                                             ChangeType type,
                                             const std::string& reason) {
    if (!PermissionRules::is_teacher(user)) {
        return {ResultCode::PermissionDenied, "Only teachers can submit change requests"};
    }

    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::RecordNotFound, "Teaching class not found"};
    }
    if (!PermissionRules::can_submit_change_request(user, item)) {
        return {ResultCode::PermissionDenied, "Not your teaching class"};
    }

    if (type == ChangeType::Cancellation) {
        const int enrolled = context_.enrollment_count(teaching_class_id);
        if (enrolled > 0 && reason.empty()) {
            return {ResultCode::InvalidInput, "Cancelling a class with " +
                                                  std::to_string(enrolled) +
                                                  " enrolled students requires a reason"};
        }
    }

    ChangeRequest request{};
    request.id = next_id_++;
    request.teaching_class_id = teaching_class_id;
    request.teacher_id = user.user_id;
    request.type = type;
    request.status = ApplyStatus::Pending;
    request.reason = reason;
    requests_.push_back(request);

    pending_notices_.push_back(std::string("Change request ") + std::to_string(request.id) +
                               " (" + change_type_name(type) + ") submitted for approval");
    return {ResultCode::Success, "Change request submitted"};
}

OperationResult ChangeRequestService::withdraw(const UserIdentity& user, int request_id) {
    ChangeRequest* request = find(request_id);
    if (request == nullptr) {
        return {ResultCode::RecordNotFound, "Change request not found"};
    }
    if (request->teacher_id != user.user_id) {
        return {ResultCode::PermissionDenied, "Not your change request"};
    }
    if (!can_transition(request->status, ApplyStatus::Withdrawn)) {
        return {ResultCode::InvalidInput,
                std::string("Cannot withdraw a request in status ") +
                    apply_status_name(request->status)};
    }
    request->status = ApplyStatus::Withdrawn;
    pending_notices_.push_back(std::string("Change request ") +
                               std::to_string(request->id) + " withdrawn");
    return {ResultCode::Success, "Change request withdrawn"};
}

OperationResult ChangeRequestService::query(const UserIdentity& user,
                                            int request_id,
                                            ChangeRequest& result) const {
    result = ChangeRequest{};
    const ChangeRequest* request = find(request_id);
    if (request == nullptr) {
        return {ResultCode::RecordNotFound, "Change request not found"};
    }
    if (user.role == Role::Teacher && request->teacher_id != user.user_id) {
        return {ResultCode::PermissionDenied, "Not your change request"};
    }
    result = *request;
    return {ResultCode::Success, "Change request found"};
}

OperationResult ChangeRequestService::list_for_teacher(
    int teacher_id, std::vector<ChangeRequest>& result) const {
    result.clear();
    for (const ChangeRequest& request : requests_) {
        if (request.teacher_id == teacher_id) result.push_back(request);
    }
    return {ResultCode::Success, "Change request list ready"};
}

}  // namespace course_selection
