#pragma once
// 2.21 US-21 Security.
// The strongest protection here is structural: the enrollment entry point does not
// take a student id, so a caller cannot act for somebody else even with a crafted
// request. What remains is checked explicitly below.
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct AuditEntry {
    int actor_id{0};
    std::string action;
    std::string subject;
};

class SecurityService {
public:
    explicit SecurityService(const SystemContext& context) : context_(context) {}

    // A student may only touch their own records; staff may act on their scope.
    OperationResult assert_can_act_on_student(const UserIdentity& actor, int student_id) const;

    // Teachers act on their own classes only; administrators on any.
    OperationResult assert_can_act_on_class(const UserIdentity& actor, int teaching_class_id) const;

    // Append-only audit trail, one line per privileged action.
    void record(const UserIdentity& actor, const std::string& action,
                const std::string& subject);
    const std::vector<AuditEntry>& audit() const { return audit_; }

private:
    const SystemContext& context_;
    std::vector<AuditEntry> audit_;
};

}  // namespace course_selection
