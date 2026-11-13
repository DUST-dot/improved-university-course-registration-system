// 2.11 US-14 review teacher requests

#include "test_support_week4.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    SystemContext context = test_support::make_context();
    ApplyService applies(context);
    ReviewService reviews(context, applies);

    const UserIdentity teacher = test_support::teacher_a();
    const UserIdentity admin = test_support::admin_user();

    // Two applications: one to approve, one to reject.
    ApplyRecord first;
    applies.create_draft(teacher, 1, "2026-2027-1", first);
    applies.submit(first.apply_id);

    ApplyRecord second;
    applies.create_draft(teacher, 3, "2026-2027-1", second);
    applies.submit(second.apply_id);

    // Only administrators decide.
    ReviewOutcome outcome;
    check_code(reviews.review(teacher, first.apply_id, true, "ok", outcome).code,
               ResultCode::PermissionDenied, "teacher cannot review");
    check(outcome.apply_id == 0, "denied review leaves the outcome cleared");

    // A rejection needs a reason, and a refused review must not change the status.
    check_code(reviews.review(admin, first.apply_id, false, "", outcome).code,
               ResultCode::InvalidInput, "rejection without a reason is refused");
    ApplyRecord still_pending;
    applies.query(first.apply_id, still_pending);
    check(still_pending.status == ApplyStatus::Pending, "status untouched after a refused review");

    // Approve.
    check_code(reviews.review(admin, first.apply_id, true, "Approved, syllabus complete",
                              outcome).code, ResultCode::Success, "administrator approves");
    check(outcome.status == ApplyStatus::Approved, "outcome reports Approved");
    check(outcome.reviewer_id == 900, "reviewer is recorded");
    check(reviews.history().size() == 1, "decision is appended to the history");

    // A decided application cannot be decided again.
    ReviewOutcome again;
    check_code(reviews.review(admin, first.apply_id, false, "second decision", again).code,
               ResultCode::InvalidInput, "a decided application cannot be reviewed twice");
    check(reviews.history().size() == 1, "failed review is not recorded");

    // Reject with a reason.
    ReviewOutcome rejected;
    check_code(reviews.review(admin, second.apply_id, false, "Missing lab hours", rejected).code,
               ResultCode::Success, "administrator rejects with a reason");
    check(rejected.status == ApplyStatus::Rejected, "outcome reports Rejected");
    check(rejected.comment == "Missing lab hours", "reason is stored");

    // Unknown id.
    check_code(reviews.review(admin, 999, true, "ok", outcome).code, ResultCode::RecordNotFound,
               "unknown application returns RecordNotFound");

    // Pending list is empty now that both were decided.
    std::vector<ApplyRecord> pending;
    check_code(reviews.pending(pending).code, ResultCode::Success, "pending list succeeds");
    check(pending.empty(), "nothing is pending");

    // One more application shows up in the pending list.
    ApplyRecord third;
    applies.create_draft(teacher, 1, "2026-2027-2", third);
    applies.submit(third.apply_id);
    reviews.pending(pending);
    check(pending.size() == 1 && pending[0].apply_id == third.apply_id,
          "pending list returns the open application");

    return test_support::finish();
}
