#include "apply_service.hpp"

#include "permission_rules.hpp"

namespace course_selection {

const char* apply_status_name(ApplyStatus status) {
    switch (status) {
        case ApplyStatus::Draft:     return "Draft";
        case ApplyStatus::Pending:   return "Pending";
        case ApplyStatus::Approved:  return "Approved";
        case ApplyStatus::Rejected:  return "Rejected";
        case ApplyStatus::Withdrawn: return "Withdrawn";
    }
    return "Unknown";
}

// 合法迁移写在一张表里，改规则只改这里
bool can_transition(ApplyStatus from, ApplyStatus to) {
    switch (from) {
        case ApplyStatus::Draft:
            return to == ApplyStatus::Pending;
        case ApplyStatus::Pending:
            return to == ApplyStatus::Approved || to == ApplyStatus::Rejected ||
                   to == ApplyStatus::Withdrawn;
        case ApplyStatus::Rejected:
            return to == ApplyStatus::Draft;      // 驳回后可以改完再提交
        case ApplyStatus::Withdrawn:
            return to == ApplyStatus::Draft;
        case ApplyStatus::Approved:
            return false;                          // 通过之后不可再变
    }
    return false;
}

ApplyRecord* ApplyService::find(int apply_id) {
    for (ApplyRecord& record : records_) {
        if (record.apply_id == apply_id) return &record;
    }
    return nullptr;
}

const ApplyRecord* ApplyService::find(int apply_id) const {
    for (const ApplyRecord& record : records_) {
        if (record.apply_id == apply_id) return &record;
    }
    return nullptr;
}

OperationResult ApplyService::create_draft(const UserIdentity& user,
                                           int course_id,
                                           const std::string& semester,
                                           ApplyRecord& created) {
    created = ApplyRecord{};   // 失败时不留上一次的结果

    if (!PermissionRules::can_open_course(user)) {
        return {ResultCode::PermissionDenied, "Only teachers can apply to open a course"};
    }
    Course course{};
    if (!context_.find_course(course_id, course)) {
        return {ResultCode::RecordNotFound, "Course not found"};
    }
    if (semester.empty()) {
        return {ResultCode::InvalidInput, "Semester must not be empty"};
    }

    ApplyRecord record{};
    record.apply_id = next_id_++;
    record.course_id = course_id;
    record.teacher_id = user.user_id;
    record.semester = semester;
    record.status = ApplyStatus::Draft;
    records_.push_back(record);
    created = record;
    return {ResultCode::Success, "Draft saved"};
}

OperationResult ApplyService::submit(int apply_id) {
    ApplyRecord* record = find(apply_id);
    if (record == nullptr) {
        return {ResultCode::RecordNotFound, "Application not found"};
    }
    if (!can_transition(record->status, ApplyStatus::Pending)) {
        return {ResultCode::InvalidInput,
                std::string("Cannot submit an application in status ") +
                    apply_status_name(record->status)};
    }
    // 唯一性：同一课程同一学期不能有两条待审批
    for (const ApplyRecord& other : records_) {
        if (other.apply_id == record->apply_id) continue;
        if (other.course_id == record->course_id && other.semester == record->semester &&
            other.status == ApplyStatus::Pending) {
            return {ResultCode::DuplicateIdentifier, "A pending application exists"};
        }
    }
    record->status = ApplyStatus::Pending;
    return {ResultCode::Success, "Application submitted"};
}

OperationResult ApplyService::withdraw(int apply_id) {
    ApplyRecord* record = find(apply_id);
    if (record == nullptr) {
        return {ResultCode::RecordNotFound, "Application not found"};
    }
    if (!can_transition(record->status, ApplyStatus::Withdrawn)) {
        return {ResultCode::InvalidInput,
                std::string("Cannot withdraw an application in status ") +
                    apply_status_name(record->status)};
    }
    record->status = ApplyStatus::Withdrawn;
    return {ResultCode::Success, "Application withdrawn"};
}

OperationResult ApplyService::review(int apply_id, bool approve, const std::string& comment) {
    ApplyRecord* record = find(apply_id);
    if (record == nullptr) {
        return {ResultCode::RecordNotFound, "Application not found"};
    }
    const ApplyStatus target = approve ? ApplyStatus::Approved : ApplyStatus::Rejected;
    if (!can_transition(record->status, target)) {
        return {ResultCode::InvalidInput,
                std::string("Cannot review an application in status ") +
                    apply_status_name(record->status)};
    }
    record->status = target;
    record->review_comment = comment;
    return {ResultCode::Success, approve ? "Application approved" : "Application rejected"};
}

OperationResult ApplyService::query(int apply_id, ApplyRecord& result) const {
    result = ApplyRecord{};
    const ApplyRecord* record = find(apply_id);
    if (record == nullptr) {
        return {ResultCode::RecordNotFound, "Application not found"};
    }
    result = *record;
    return {ResultCode::Success, "Application found"};
}

OperationResult ApplyService::list_for_teacher(int teacher_id,
                                               std::vector<ApplyRecord>& result) const {
    result.clear();
    for (const ApplyRecord& record : records_) {
        if (record.teacher_id == teacher_id) result.push_back(record);
    }
    return {ResultCode::Success, "Application list ready"};
}

}  // namespace course_selection
