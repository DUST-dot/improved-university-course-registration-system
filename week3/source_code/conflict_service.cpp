#include "conflict_service.hpp"

#include <cstdio>

namespace course_selection {

bool intervals_overlap(int start_a, int end_a, int start_b, int end_b) {
    // Left-closed right-open: 09:00-10:00 and 10:00-11:00 are adjacent, not clashing.
    return start_a < end_b && start_b < end_a;
}

bool weeks_compatible(WeekType a, WeekType b) {
    if (a == WeekType::All || b == WeekType::All) return true;
    return a == b;
}

bool meetings_clash(const ClassMeeting& a, const ClassMeeting& b) {
    return a.day_of_week == b.day_of_week &&
           weeks_compatible(a.week_type, b.week_type) &&
           intervals_overlap(a.start_minute, a.end_minute, b.start_minute, b.end_minute);
}

std::string format_meeting(const ClassMeeting& meeting) {
    static const char* names[] = {"", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d-%02d:%02d",
                  meeting.start_minute / 60, meeting.start_minute % 60,
                  meeting.end_minute / 60, meeting.end_minute % 60);
    const char* week = meeting.week_type == WeekType::Odd   ? " odd"
                     : meeting.week_type == WeekType::Even  ? " even"
                                                            : " all";
    const int day = (meeting.day_of_week >= 1 && meeting.day_of_week <= 7)
                        ? meeting.day_of_week
                        : 0;
    return std::string(names[day]) + " " + buffer + week;
}

OperationResult ConflictService::detect(int student_id,
                                        int course_id,
                                        std::vector<Conflict>& conflicts) const {
    conflicts.clear();

    Course target{};
    if (!context_.find_course(course_id, target)) {
        return {ResultCode::RecordNotFound, "Course not found"};
    }

    // Courses the student already holds as Active records. Meetings live on the
    // teaching class, so they are flattened into a small local view instead of
    // being pushed back into Course (Course has no meetings of its own).
    struct HeldCourse {
        int course_id;
        std::string course_code;
        std::vector<ClassMeeting> meetings;
    };
    std::vector<HeldCourse> taken;
    for (const Enrollment& record : context_.active_enrollments(student_id)) {
        TeachingClass item{};
        if (!context_.find_teaching_class(record.teaching_class_id, item)) continue;
        Course course{};
        if (!context_.find_course(item.course_id, course)) continue;
        taken.push_back(HeldCourse{course.course_id, course.course_code, item.meetings});
    }

    // Case 3: prerequisites. Reported first because it is the cheapest to explain.
    for (int required : target.prerequisite_ids) {
        bool satisfied = false;
        for (const HeldCourse& held : taken) {
            if (held.course_id == required) {
                satisfied = true;
                break;
            }
        }
        if (!satisfied) {
            Conflict conflict;
            conflict.kind = Conflict::Kind::MissingPrerequisite;
            conflict.other_course_id = required;
            conflict.message = "Missing prerequisite: course id " + std::to_string(required) +
                               " is required before " + target.course_code;
            conflicts.push_back(conflict);
        }
    }

    // Cases 1 and 2: one pass over the active courses. The caller validates a
    // course, not a class, so every class of that course contributes its meetings.
    std::vector<ClassMeeting> target_meetings;
    for (const TeachingClass& item : context_.teaching_classes()) {
        if (item.course_id == course_id) {
            target_meetings.insert(target_meetings.end(), item.meetings.begin(),
                                   item.meetings.end());
        }
    }

    for (const HeldCourse& held : taken) {
        if (held.course_id == target.course_id) continue;   // never compare with itself

        bool clash = false;
        ClassMeeting mine{};
        ClassMeeting theirs{};
        for (const ClassMeeting& a : target_meetings) {
            for (const ClassMeeting& b : held.meetings) {
                if (meetings_clash(a, b)) {
                    clash = true;
                    mine = a;
                    theirs = b;
                    break;
                }
            }
            if (clash) break;
        }
        if (!clash) continue;

        Conflict conflict;
        conflict.kind = Conflict::Kind::TimeClash;
        conflict.other_course_id = held.course_id;
        conflict.message = "Time clash: " + target.course_code + " (" + format_meeting(mine) +
                           ") overlaps " + held.course_code + " (" +
                           format_meeting(theirs) + ")";
        conflicts.push_back(conflict);
    }

    return {ResultCode::Success, conflicts.empty() ? "No conflict" : "Conflicts found"};
}

OperationResult ConflictService::check_before_submit(int student_id,
                                                     int course_id,
                                                     std::string& detail) const {
    detail.clear();
    std::vector<Conflict> conflicts;
    const OperationResult found = detect(student_id, course_id, conflicts);
    if (!ok(found)) return found;
    if (conflicts.empty()) return {ResultCode::Success, "No conflict"};

    // Take the first conflict as the reason, but keep the full text so the UI can
    // list every clash if it wants to.
    for (const Conflict& conflict : conflicts) {
        if (!detail.empty()) detail += "; ";
        detail += conflict.message;
    }
    const bool prerequisite = conflicts.front().kind == Conflict::Kind::MissingPrerequisite;
    return {prerequisite ? ResultCode::InvalidInput : ResultCode::TimeConflict, detail};
}

}  // namespace course_selection
