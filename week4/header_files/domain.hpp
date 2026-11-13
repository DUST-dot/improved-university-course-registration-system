#pragma once

// Shared domain model for every iteration.
// Week types and prerequisite lists exist because the conflict check (US-04) needs
// them; the other fields are the ones the services agreed on.

#include <string>
#include <vector>

namespace course_selection {

// Odd and even weeks never meet, so a clash check must look at this.
enum class WeekType { All, Odd, Even };

struct ClassMeeting {
    int day_of_week{1};          // 1 = Monday
    int start_minute{0};         // minutes from midnight
    int end_minute{0};           // left-closed, right-open
    WeekType week_type{WeekType::All};
    std::string room;
};

struct Course {
    int course_id{0};
    std::string course_code;
    std::string course_name;
    double credits{0.0};         // half points are legal, so double
    std::vector<int> prerequisite_ids;   // course ids that must be passed first
};

// A course can open several teaching classes; capacity belongs to the class.
struct TeachingClass {
    int teaching_class_id{0};
    int course_id{0};
    int teacher_id{0};
    int capacity{0};
    std::string time_slot;       // display form, e.g. MON-1-2
    std::string room;
    std::string semester;
    std::vector<ClassMeeting> meetings;   // detailed form, used by clash and timetable
};

struct Student {
    int student_id{0};
    std::string student_number;
    std::string name;
    std::string major;
    bool account_active{true};
};

struct Teacher {
    int teacher_id{0};
    std::string teacher_number;
    std::string name;
    std::string department;
    bool account_active{true};
};

struct Administrator {
    int administrator_id{0};
    std::string account_number;
    std::string name;
    bool account_active{true};
};

enum class EnrollmentStatus { Active, Dropped };

struct Enrollment {
    int student_id{0};
    int teaching_class_id{0};
    EnrollmentStatus status{EnrollmentStatus::Active};
};

enum class Role { Student, Teacher, Administrator };

// What the login module hands to every service. The id is shared with the three
// people tables so no mapping step is needed.
struct UserIdentity {
    int user_id{0};
    Role role{Role::Student};
    std::string account_number;
    std::string display_name;
};

struct RegistrationRound {
    std::string opens_at;
    std::string closes_at;
};

struct RegistrationRule {
    std::string opens_at;             // empty means "use the manual switch"
    std::string closes_at;
    bool registration_open{false};
    int maximum_credits{25};
};

// Failure reasons. Codes are for logs, messages are for people.
enum class ResultCode {
    Success = 0,
    RegistrationClosed,
    AlreadyEnrolled,
    TimeConflict,
    CreditLimitExceeded,
    TeachingClassFull,
    TeachingClassNotFound,
    ActiveEnrollmentNotFound,
    PermissionDenied,
    InvalidInput,
    RecordNotFound,
    DuplicateIdentifier,
    AuthenticationFailed,
    AccountLocked,
    AccountDisabled,
    SessionExpired,
    SessionInvalid
};

struct OperationResult {
    ResultCode code{ResultCode::Success};
    std::string message;
};

inline bool ok(const OperationResult& result) {
    return result.code == ResultCode::Success;
}

inline const char* result_code_name(ResultCode code) {
    switch (code) {
        case ResultCode::Success:                  return "Success";
        case ResultCode::RegistrationClosed:       return "RegistrationClosed";
        case ResultCode::AlreadyEnrolled:          return "AlreadyEnrolled";
        case ResultCode::TimeConflict:             return "TimeConflict";
        case ResultCode::CreditLimitExceeded:      return "CreditLimitExceeded";
        case ResultCode::TeachingClassFull:        return "TeachingClassFull";
        case ResultCode::TeachingClassNotFound:    return "TeachingClassNotFound";
        case ResultCode::ActiveEnrollmentNotFound: return "ActiveEnrollmentNotFound";
        case ResultCode::PermissionDenied:         return "PermissionDenied";
        case ResultCode::InvalidInput:             return "InvalidInput";
        case ResultCode::RecordNotFound:           return "RecordNotFound";
        case ResultCode::DuplicateIdentifier:      return "DuplicateIdentifier";
        case ResultCode::AuthenticationFailed:     return "AuthenticationFailed";
        case ResultCode::AccountLocked:            return "AccountLocked";
        case ResultCode::AccountDisabled:          return "AccountDisabled";
        case ResultCode::SessionExpired:           return "SessionExpired";
        case ResultCode::SessionInvalid:           return "SessionInvalid";
    }
    return "Unknown";
}

}  // namespace course_selection
