#include "security_service.hpp"

#include "permission_rules.hpp"

namespace course_selection {

OperationResult SecurityService::assert_can_act_on_student(const UserIdentity& actor,
                                                           int student_id) const {
    if (PermissionRules::is_student(actor) && actor.user_id != student_id) {
        return {ResultCode::PermissionDenied, "A student may only act on their own records"};
    }
    return {ResultCode::Success, "Allowed"};
}

OperationResult SecurityService::assert_can_act_on_class(const UserIdentity& actor,
                                                         int teaching_class_id) const {
    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::RecordNotFound, "Teaching class not found"};
    }
    if (!PermissionRules::can_view_teaching_class(actor, item)) {
        return {ResultCode::PermissionDenied, "Not your teaching class"};
    }
    return {ResultCode::Success, "Allowed"};
}

void SecurityService::record(const UserIdentity& actor, const std::string& action,
                             const std::string& subject) {
    audit_.push_back(AuditEntry{actor.user_id, action, subject});
}

}  // namespace course_selection
