#pragma once

// ============================================================================
// 教师端查询（US-09）
//
// 教师登录后能做的第一件事就是看自己的教学班。本模块只读，
// 不改任何记录；人数与余量全部从 SystemContext 现算，不另存计数。
//
// 三条越权防线，缺一不可：
//   1. 入口 require_role：学生账号进不来；
//   2. 列表按 teacher_id 过滤：别人的班根本不出现在结果里；
//   3. 单个教学班再查一次 can_view_teaching_class：即使 id 猜对也读不到。
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
    std::vector<RosterEntry> students;  // 只含 Active 记录，按学号排序
};

class TeacherService {
public:
    explicit TeacherService(const SystemContext& context);

    // 列出本人负责的教学班；管理员调用时列出全部。
    OperationResult assigned_teaching_classes(const UserIdentity& user,
                                              std::vector<TeachingClassSummary>& result) const;

    // 单个教学班的容量、人数与余量；越权返回 PermissionDenied。
    OperationResult class_summary(const UserIdentity& user,
                                  int teaching_class_id,
                                  TeachingClassSummary& result) const;

    // 学生名单：只列 Active 记录，已退课的学生不出现。
    OperationResult class_roster(const UserIdentity& user,
                                 int teaching_class_id,
                                 ClassRoster& result) const;

private:
    TeachingClassSummary build_summary(const TeachingClass& item) const;

    const SystemContext& context_;
};

}  // namespace course_selection
