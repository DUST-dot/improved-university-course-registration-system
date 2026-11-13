// US-10 section
//

#include "test_support_week2.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    ApplyService service(context);

    const UserIdentity teacher = test_support::teacher_a();
    const UserIdentity other_teacher = test_support::teacher_b();
    const UserIdentity student = test_support::student_user();

    check(can_transition(ApplyStatus::Draft, ApplyStatus::Pending), "Draft -> Pending allowed");
    check(!can_transition(ApplyStatus::Draft, ApplyStatus::Approved),
          "Draft -> Approved is not allowed");
    check(can_transition(ApplyStatus::Pending, ApplyStatus::Approved), "Pending -> Approved");
    check(can_transition(ApplyStatus::Pending, ApplyStatus::Rejected), "Pending -> Rejected");
    check(can_transition(ApplyStatus::Pending, ApplyStatus::Withdrawn), "Pending -> Withdrawn");
    check(can_transition(ApplyStatus::Rejected, ApplyStatus::Draft), "Rejected -> Draft");
    check(!can_transition(ApplyStatus::Approved, ApplyStatus::Draft),
          "Approved is terminal");

    ApplyRecord draft;
    check_code(service.create_draft(teacher, 1, "2026-2027-1", draft).code, ResultCode::Success,
               "teacher creates a draft");
    check(draft.apply_id == 1, "draft gets an id");
    check(draft.status == ApplyStatus::Draft, "new record starts as Draft");
    check(draft.teacher_id == 100, "draft records the applicant");
    check(draft.course_id == 1, "draft records the course");
    check(draft.review_comment.empty(), "draft has no review comment yet");

    ApplyRecord denied;
    check_code(service.create_draft(student, 1, "2026-2027-1", denied).code,
               ResultCode::PermissionDenied, "student cannot apply to open a course");
    check(denied.apply_id == 0, "denied draft leaves the output cleared");

    ApplyRecord invalid;
    check_code(service.create_draft(teacher, 999, "2026-2027-1", invalid).code,
               ResultCode::RecordNotFound, "unknown course is rejected");
    check_code(service.create_draft(teacher, 1, "", invalid).code, ResultCode::InvalidInput,
               "empty semester is rejected");
    check(service.size() == 1, "only the valid draft was stored");

    check_code(service.submit(draft.apply_id).code, ResultCode::Success, "draft can be submitted");
    ApplyRecord current;
    check_code(service.query(draft.apply_id, current).code, ResultCode::Success, "query after submit");
    check(current.status == ApplyStatus::Pending, "status is Pending after submit");

    check_code(service.submit(draft.apply_id).code, ResultCode::InvalidInput,
               "submitting twice is rejected");
    check(service.query(draft.apply_id, current).code == ResultCode::Success &&
              current.status == ApplyStatus::Pending,
          "failed submit left the status untouched");

    ApplyRecord second;
    service.create_draft(teacher, 1, "2026-2027-1", second);
    check_code(service.submit(second.apply_id).code, ResultCode::DuplicateIdentifier,
               "a second pending application for the same course is rejected");

    ApplyRecord other_semester;
    service.create_draft(teacher, 1, "2026-2027-2", other_semester);
    check_code(service.submit(other_semester.apply_id).code, ResultCode::Success,
               "another semester is not a duplicate");

    check_code(service.withdraw(other_semester.apply_id).code, ResultCode::Success,
               "pending application can be withdrawn");
    check(service.query(other_semester.apply_id, current).code == ResultCode::Success &&
              current.status == ApplyStatus::Withdrawn,
          "status is Withdrawn");
    check_code(service.withdraw(other_semester.apply_id).code, ResultCode::InvalidInput,
               "withdrawing twice is rejected");

    check_code(service.review(draft.apply_id, true, "Approved by the office").code,
               ResultCode::Success, "administrator approves");
    check(service.query(draft.apply_id, current).code == ResultCode::Success &&
              current.status == ApplyStatus::Approved,
          "status is Approved");
    check(current.review_comment == "Approved by the office", "review comment is stored");
    check_code(service.review(draft.apply_id, false, "again").code, ResultCode::InvalidInput,
               "an approved application cannot be reviewed again");

    ApplyRecord reject_target;
    service.create_draft(teacher, 2, "2026-2027-1", reject_target);
    service.submit(reject_target.apply_id);
    check_code(service.review(reject_target.apply_id, false, "Incomplete syllabus").code,
               ResultCode::Success, "administrator rejects");
    check(service.query(reject_target.apply_id, current).code == ResultCode::Success &&
              current.status == ApplyStatus::Rejected && current.review_comment == "Incomplete syllabus",
          "rejection stores its reason");
    check_code(service.submit(reject_target.apply_id).code, ResultCode::InvalidInput,
               "a rejected application must go back to Draft before resubmitting");

    ApplyRecord missing;
    check_code(service.query(999, missing).code, ResultCode::RecordNotFound,
               "unknown application returns RecordNotFound");
    check(missing.apply_id == 0, "failed query clears the result");

    std::vector<ApplyRecord> mine;
    check_code(service.list_for_teacher(100, mine).code, ResultCode::Success,
               "list for teacher succeeds");
    check(mine.size() == 4, "teacher A owns four applications");
    std::vector<ApplyRecord> others;
    check_code(service.list_for_teacher(200, others).code, ResultCode::Success,
               "list for another teacher succeeds");
    check(others.empty(), "teacher B owns none");

    std::cout << "  apply_status_name(Approved) = " << apply_status_name(ApplyStatus::Approved)
              << '\n';
    return test_support::finish();
}
