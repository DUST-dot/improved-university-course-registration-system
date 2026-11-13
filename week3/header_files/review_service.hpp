#pragma once

// ============================================================================
// 2.11 US-14 Review teacher requests
//
// Approval is the moment a decision becomes final, so two failures have to be
// impossible: reviewing the same request twice, and reviewing something that is
// not yours to review. The state machine in ApplyService already rejects the
// first; this service adds the permission gate in front of it and records who
// decided and why.
// ============================================================================

#include <string>
#include <vector>

#include "apply_service.hpp"
#include "system_context.hpp"

namespace course_selection {

struct ReviewOutcome {
    int apply_id{0};
    ApplyStatus status{ApplyStatus::Draft};
    std::string comment;
    int reviewer_id{0};
};

class ReviewService {
public:
    ReviewService(SystemContext& context, ApplyService& applies)
        : context_(context), applies_(applies) {}

    // approve = true means approve, false means reject. A rejection must carry a
    // reason: the teacher has to know what to fix before resubmitting.
    OperationResult review(const UserIdentity& user,
                           int apply_id,
                           bool approve,
                           const std::string& comment,
                           ReviewOutcome& outcome);

    // Requests still waiting for a decision, oldest first.
    OperationResult pending(std::vector<ApplyRecord>& result) const;

    const std::vector<ReviewOutcome>& history() const { return history_; }

private:
    SystemContext& context_;
    ApplyService& applies_;
    std::vector<ReviewOutcome> history_;
};

}  // namespace course_selection
