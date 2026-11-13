#pragma once
// 2.17 US-07 Check the class schedule.
// Built from active enrollments plus the class meetings; no second copy of a
// timetable is ever stored.
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct TimetableEntry {
    int teaching_class_id{0};
    std::string course_code;
    std::string course_name;
    std::string teacher_name;
    ClassMeeting meeting;
};

class TimetableService {
public:
    explicit TimetableService(const SystemContext& context) : context_(context) {}

    // Sorted by day, then by start minute. Only students have a timetable.
    OperationResult for_student(const UserIdentity& user,
                                std::vector<TimetableEntry>& result) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
