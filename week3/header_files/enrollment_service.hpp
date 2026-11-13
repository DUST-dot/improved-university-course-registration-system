#pragma once

// ============================================================================
// 2.13 US-06 Drop class (and the feedback path for US-05)
//
// Enroll and drop are the two halves of the same operation, so they share one
// gateway here: both go through the same window check and the same seat counter.
// A drop never deletes the record; it flips the status to Dropped, which keeps the
// history and lets the roster and the seat count stay consistent.
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct EnrollOutcome {
    int teaching_class_id{0};
    int active_enrollments{0};
    int remaining_seats{0};
    std::string message;   // ready to show to the student
};

class EnrollmentService {
public:
    explicit EnrollmentService(SystemContext& context) : context_(context) {}

    // Checks window, class existence, capacity, duplicates and the credit cap,
    // in that order, so the student is told the outermost problem first.
    OperationResult enroll(int student_id, int teaching_class_id, EnrollOutcome& outcome);

    // Only an Active record can be dropped. The record is kept with status Dropped.
    OperationResult drop(int student_id, int teaching_class_id, EnrollOutcome& outcome);

    // Total active credits of a student, used by the credit cap check.
    double active_credits(int student_id) const;

private:
    OperationResult describe(int teaching_class_id, EnrollOutcome& outcome,
                             const std::string& message) const;

    SystemContext& context_;
};

}  // namespace course_selection
