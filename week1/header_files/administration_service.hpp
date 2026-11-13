#pragma once

// ============================================================================
// 教务后台维护（US-13）
//
// 本周只做与登录直接相关的三件事：改学分上限、设选课窗口、关闭选课。
// 课程与人员的增删改留到 UC13/UC16，本文件先不铺开。
//
// 每个写操作都走同一条顺序：先校验角色，再校验输入，最后才改数据。
// 少一步就会出现"权限没有但数据改了"或者"参数非法却写进去了"。
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct RegistrationRoundStatus {
    bool opened{false};       // 本轮是否已经开放过
    std::string opens_at{};
    std::string closes_at{};
};

class AdministrationService {
public:
    explicit AdministrationService(SystemContext& context);

    // 学分上限只能在"本轮尚未开放"时修改。
    // 否则管理员可以先把窗口关掉、调高上限、再打开，已经选完课的学生就被超发了学分。
    OperationResult set_maximum_credits(const UserIdentity& user, int maximum_credits);

    // 选课窗口：开始必须早于结束，格式必须是 YYYY-MM-DDTHH:MM。
    OperationResult set_registration_period(const UserIdentity& user,
                                            const std::string& opens_at,
                                            const std::string& closes_at);

    OperationResult close_registration(const UserIdentity& user);

    // 只读：给界面显示当前轮次状态。
    RegistrationRoundStatus current_round() const;

private:
    SystemContext& context_;
};

}  // namespace course_selection
