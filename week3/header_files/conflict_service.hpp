#pragma once

// ============================================================================
// 2.10 US-04 Time conflict reminder
//
// Adapted from the standalone `schedule_conflict.cpp` demonstration into the
// unified model (course_selection namespace, SystemContext, OperationResult).
//
// Three cases have to be covered, and leaving any of them out is what made the
// original implementation come back for rework:
//   1. the same course already has another teaching class enrolled;
//   2. two classes overlap in time (day plus interval), and the week type must be
//      compatible, otherwise an odd-week and an even-week class never clash;
//   3. a prerequisite course has not been passed yet.
//
// Time clash and missing prerequisite are reported separately: they are different
// problems and a student should not have to guess which one blocked the request.
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct Conflict {
    enum class Kind { TimeClash, MissingPrerequisite };

    Kind kind{Kind::TimeClash};
    int other_course_id{0};
    std::string message;   // ready to show to the student
};

// Interval overlap on a left-closed right-open basis: adjacent slots do not clash.
bool intervals_overlap(int start_a, int end_a, int start_b, int end_b);

// All-week is compatible with anything; odd/odd and even/even are compatible;
// odd/even never meet.
bool weeks_compatible(WeekType a, WeekType b);

// One pair of meetings clashes when day, week type and interval all line up.
bool meetings_clash(const ClassMeeting& a, const ClassMeeting& b);

// "MON 09:00-10:00 odd week" style label used in conflict messages.
std::string format_meeting(const ClassMeeting& meeting);

class ConflictService {
public:
    explicit ConflictService(const SystemContext& context) : context_(context) {}

    // Read-only. `student_id` supplies the already active enrollments; the course
    // being checked is passed separately so a draft can be validated before submit.
    OperationResult detect(int student_id,
                           int course_id,
                           std::vector<Conflict>& conflicts) const;

    // Convenience wrapper used by the submit path: returns TimeConflict or
    // InvalidInput (missing prerequisite) with a message ready for the UI.
    OperationResult check_before_submit(int student_id,
                                        int course_id,
                                        std::string& detail) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
