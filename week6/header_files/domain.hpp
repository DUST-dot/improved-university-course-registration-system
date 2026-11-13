#pragma once

// ============================================================================
// 统一领域模型（week2 / week3 共用）
//
// 合并了两套既有代码：
//   - week1 的类型命名与 ResultCode（course_id / credits / teaching_class_id / 完整失败码）
//   - 2.5–2.9 的扩展字段（TeachingClass 的 time_slot / room / semester、RegistrationRound、
//     UserIdentity 的 display_name）
//
// 字段顺序刻意与两边的聚合初始化都兼容：
//   Course{1, "C001", "Data Structures", 4.0}
//   Student{1000, "20230001", "Zhang San"}
//   TeachingClass{10, 1, 100, 2, "MON-1-2", "A101", "2026-2027-1"}
//   UserIdentity{100, Role::Teacher, "t100", "Teacher A"}
// ============================================================================

#include <string>
#include <vector>

namespace course_selection {

// 单双周：全周 / 单周 / 双周。冲突判定要区分，否则单双周的两门课会被误判为撞课。
enum class WeekType { All, Odd, Even };

struct ClassMeeting {
    int day_of_week{1};
    int start_minute{0};
    int end_minute{0};
    WeekType week_type{WeekType::All};
    std::string room;
};

struct Course {
    int course_id{0};
    std::string course_code;
    std::string course_name;
    double credits{0.0};   // 0.5 步长，所以用 double
    // 先修课：这些课程必须先通过（有 Active 记录）才能选本课程
    std::vector<int> prerequisite_ids;
};

// 一个 Course 可以开多个教学班；容量属于教学班。
// time_slot/room/semester 是 2.5–2.9 需要的展示字段；
// meetings 是更细的上课时间，供课表与冲突判定使用，两者可以并存。
struct TeachingClass {
    int teaching_class_id{0};
    int course_id{0};
    int teacher_id{0};
    int capacity{0};
    std::string time_slot;
    std::string room;
    std::string semester;
    std::vector<ClassMeeting> meetings;
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

// 登录成功后交给业务模块的身份。user_id 与三个角色表的编号同一套。
struct UserIdentity {
    int user_id{0};
    Role role{Role::Student};
    std::string account_number;   // 学号 / 工号 / 管理员账号
    std::string display_name;
};

// 一个选课轮次（US-16 用来判重叠）。
struct RegistrationRound {
    std::string opens_at;
    std::string closes_at;
};

struct RegistrationRule {
    std::string opens_at;                 // 空表示未设置，沿用人工开关
    std::string closes_at;
    bool registration_open{false};        // 人工开关
    int maximum_credits{25};              // US-13：学分上限
};

// 失败原因。日志用码，界面用 message。
enum class ResultCode {
    Success = 0,
    // 选课与查询
    RegistrationClosed,
    AlreadyEnrolled,
    TimeConflict,
    CreditLimitExceeded,
    TeachingClassFull,
    TeachingClassNotFound,
    ActiveEnrollmentNotFound,
    // 通用
    PermissionDenied,
    InvalidInput,
    RecordNotFound,
    DuplicateIdentifier,
    // 登录（week1）
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
        case ResultCode::Success:               return "Success";
        case ResultCode::RegistrationClosed:    return "RegistrationClosed";
        case ResultCode::AlreadyEnrolled:       return "AlreadyEnrolled";
        case ResultCode::TimeConflict:          return "TimeConflict";
        case ResultCode::CreditLimitExceeded:   return "CreditLimitExceeded";
        case ResultCode::TeachingClassFull:     return "TeachingClassFull";
        case ResultCode::TeachingClassNotFound: return "TeachingClassNotFound";
        case ResultCode::ActiveEnrollmentNotFound: return "ActiveEnrollmentNotFound";
        case ResultCode::PermissionDenied:      return "PermissionDenied";
        case ResultCode::InvalidInput:          return "InvalidInput";
        case ResultCode::RecordNotFound:        return "RecordNotFound";
        case ResultCode::DuplicateIdentifier:   return "DuplicateIdentifier";
        case ResultCode::AuthenticationFailed:  return "AuthenticationFailed";
        case ResultCode::AccountLocked:         return "AccountLocked";
        case ResultCode::AccountDisabled:       return "AccountDisabled";
        case ResultCode::SessionExpired:        return "SessionExpired";
        case ResultCode::SessionInvalid:        return "SessionInvalid";
    }
    return "Unknown";
}

}  // namespace course_selection
