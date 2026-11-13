#pragma once

// ============================================================================
// US-09 section
//
//
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct TeachingClassSummary {
    int teaching_class_id;
    int course_id;
    std::string course_code;
    std::string course_name;
    int credits;
    int teacher_id;
    std::string teacher_name;
    int capacity;
    int active_enrollments;
    int remaining_seats;
    std::vector<ClassMeeting> meetings;
};

struct RosterEntry {
    int student_id;
    std::string student_number;
    std::string name;
};

struct ClassRoster {
    int teaching_class_id;
    int capacity;
    int active_enrollments;
    int remaining_seats;
    std::vector<RosterEntry> students;
};

class TeacherService {
public:
    explicit TeacherService(const SystemContext& context);

    OperationResult assigned_teaching_classes(const UserIdentity& user,
                                              std::vector<TeachingClassSummary>& result) const;

    OperationResult class_summary(const UserIdentity& user,
                                  int teaching_class_id,
                                  TeachingClassSummary& result) const;

    OperationResult class_roster(const UserIdentity& user,
                                 int teaching_class_id,
                                 ClassRoster& result) const;

private:
    TeachingClassSummary build_summary(const TeachingClass& item) const;

    const SystemContext& context_;
};

}  // namespace course_selection
