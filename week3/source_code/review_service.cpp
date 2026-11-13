#include "review_service.hpp"

#include "permission_rules.hpp"

namespace course_selection {

OperationResult ReviewService::review(const UserIdentity& user,
                                      int apply_id,
                                      bool approve,
                                      const std::string& comment,
                                      ReviewOutcome& outcome) {
    outcome = ReviewOutcome{};

    // 1. Only administrators decide.
    if (!PermissionRules::can_review_application(user)) {
        return {ResultCode::PermissionDenied, "Only administrators can review requests"};
    }
    // 2. A rejection without a reason is useless to the applicant.
    if (!approve && comment.empty()) {
        return {ResultCode::InvalidInput, "A rejection must carry a reason"};
    }

    ApplyRecord before{};
    const OperationResult fetched = applies_.query(apply_id, before);
    if (!ok(fetched)) return fetched;

    // 3. The state machine rejects a second decision; ApplyService owns that rule.
    const OperationResult decided = applies_.review(apply_id, approve, comment);
    if (!ok(decided)) return decided;

    ApplyRecord after{};
    applies_.query(apply_id, after);

    outcome.apply_id = apply_id;
    outcome.status = after.status;
    outcome.comment = comment;
    outcome.reviewer_id = user.user_id;
    history_.push_back(outcome);
    return decided;
}

OperationResult ReviewService::pending(std::vector<ApplyRecord>& result) const {
    result.clear();
    for (int id = 1;; ++id) {
        ApplyRecord record{};
        if (!ok(applies_.query(id, record))) {
            // ApplyService assigns ids from 1 upwards without gaps, so the first
            // miss means the end of the list.
            if (record.apply_id == 0) break;
            continue;
        }
        if (record.status == ApplyStatus::Pending) result.push_back(record);
    }
    return {ResultCode::Success, "Pending requests ready"};
}

}  // namespace course_selection
