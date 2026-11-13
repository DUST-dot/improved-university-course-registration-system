#pragma once

// ============================================================================
// US-10 section
//
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

enum class ApplyStatus { Draft, Pending, Approved, Rejected, Withdrawn };

const char* apply_status_name(ApplyStatus status);

bool can_transition(ApplyStatus from, ApplyStatus to);

struct ApplyRecord {
    int apply_id{0};
    int course_id{0};
    int teacher_id{0};
    std::string semester;
    ApplyStatus status{ApplyStatus::Draft};
    std::string review_comment;
};

class ApplyService {
public:
    explicit ApplyService(SystemContext& context) : context_(context) {}

    OperationResult create_draft(const UserIdentity& user,
                                 int course_id,
                                 const std::string& semester,
                                 ApplyRecord& created);

    OperationResult submit(int apply_id);

    OperationResult withdraw(int apply_id);

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
