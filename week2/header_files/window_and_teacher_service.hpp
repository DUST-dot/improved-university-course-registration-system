#pragma once

// ============================================================================
// US-16 section
//
// US-09 section
// US-11 section
//
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

// ---------------------------------------------------------------------------
// US-16 section
// ---------------------------------------------------------------------------

enum class WindowState { NotStarted, Open, Closed };

const char* window_state_name(WindowState state);

WindowState state_of(const RegistrationRule& rule, const std::string& now);

class RegistrationWindowService {
public:
    explicit RegistrationWindowService(SystemContext& context) : context_(context) {}

    OperationResult set_registration_period(const UserIdentity& user,
                                            const std::string& opens_at,
                                            const std::string& closes_at);

    OperationResult set_manual_switch(const UserIdentity& user, bool open);

    OperationResult current_rule(RegistrationRule& result) const;
    OperationResult window_state(WindowState& result) const;
    OperationResult window_state_at(const std::string& now, WindowState& result) const;

    const std::vector<RegistrationRound>& rounds() const { return context_.registration_rounds(); }

private:
    SystemContext& context_;
};

// ---------------------------------------------------------------------------
// US-11 section
// ---------------------------------------------------------------------------

struct TeachingClassSummary {
    int teaching_class_id{0};
    int course_id{0};
    std::string course_code;
    std::string course_name;
    int capacity{0};
    int active_enrollments{0};
    int remaining_seats{0};
};

struct RosterEntry {
    int student_id{0};
    std::string student_number;
    std::string name;
};

struct ClassRoster {
    int teaching_class_id{0};
    std::vector<RosterEntry> students;
};

class TeacherService {
public:
    explicit TeacherService(const SystemContext& context) : context_(context) {}

// US-09 section
    OperationResult assigned_teaching_classes(const UserIdentity& user,
                                              std::vector<TeachingClassSummary>& result) const;

// US-11 section
    OperationResult class_summary(const UserIdentity& user,
                                  int teaching_class_id,
                                  TeachingClassSummary& result) const;

// US-11 section
    OperationResult class_roster(const UserIdentity& user,
                                 int teaching_class_id,
                                 ClassRoster& result) const;

private:
    TeachingClassSummary build_summary(const TeachingClass& item) const;

    const SystemContext& context_;
};

}  // namespace course_selection
