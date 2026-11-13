#pragma once

// ============================================================================
// 2.7 US-16 选课窗口 + 2.8 US-11 教师端（合并自两套代码，适配统一模型）
//
// 合并说明：
//   - RegistrationWindow 部分来自 se-course-system-tasks 的 registration_window
//     （parse_iso_minute / moment_before / state_of 已上移到 time_util.hpp）
//   - TeacherService 部分把 week1 的 assigned_teaching_classes（US-09）与
//     2.5–2.9 的 class_summary / class_roster（US-11）合成一个类
//
// 窗口状态不落库：由 now 与两个时刻现算，省掉定时任务。
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

// ---------------------------------------------------------------------------
// US-16 选课窗口
// ---------------------------------------------------------------------------

enum class WindowState { NotStarted, Open, Closed };

const char* window_state_name(WindowState state);

// 两个时刻都不为空时按时间算，否则退回管理员手动开关
WindowState state_of(const RegistrationRule& rule, const std::string& now);

class RegistrationWindowService {
public:
    explicit RegistrationWindowService(SystemContext& context) : context_(context) {}

    OperationResult set_registration_period(const UserIdentity& user,
                                            const std::string& opens_at,
                                            const std::string& closes_at);

    // 未设置时刻时用的人工开关
    OperationResult set_manual_switch(const UserIdentity& user, bool open);

    OperationResult current_rule(RegistrationRule& result) const;
    OperationResult window_state(WindowState& result) const;
    OperationResult window_state_at(const std::string& now, WindowState& result) const;

    const std::vector<RegistrationRound>& rounds() const { return context_.registration_rounds(); }

private:
    SystemContext& context_;
};

// ---------------------------------------------------------------------------
// US-11 教师端（含 week1 的 US-09 列表接口）
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

    // US-09：列出本人负责的教学班（管理员调用时列出全部）
    OperationResult assigned_teaching_classes(const UserIdentity& user,
                                              std::vector<TeachingClassSummary>& result) const;

    // US-11：单个教学班的容量、人数与余量
    OperationResult class_summary(const UserIdentity& user,
                                  int teaching_class_id,
                                  TeachingClassSummary& result) const;

    // US-11：只收 Active 记录，已退课的学生不出现
    OperationResult class_roster(const UserIdentity& user,
                                 int teaching_class_id,
                                 ClassRoster& result) const;

private:
    TeachingClassSummary build_summary(const TeachingClass& item) const;

    const SystemContext& context_;
};

}  // namespace course_selection
