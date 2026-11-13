#include "administration_service.hpp"

#include <cstdio>
#include <string>

#include "permission_rules.hpp"

namespace course_selection {
namespace {

// 判断 "YYYY-MM-DDTHH:MM" 是否可用，并顺带取出可比较的整数键：
// 年月日时分变成 YYYYMMDDHHMM 这样的 12 位数字，直接比大小就等于比时间。
bool parse_iso_key(const std::string& text, long long& key) {
    int year = 0;
    unsigned month = 0, day = 0, hour = 0, minute = 0;
    if (std::sscanf(text.c_str(), "%4d-%2u-%2uT%2u:%2u", &year, &month, &day,
                    &hour, &minute) != 5) {
        return false;
    }
    if (month == 0 || month > 12 || day == 0 || day > 31 || hour > 23 || minute > 59) {
        return false;
    }
    key = ((((static_cast<long long>(year) * 100 + month) * 100 + day) * 100 + hour) * 100) +
          minute;
    return true;
}

}  // namespace

AdministrationService::AdministrationService(SystemContext& context) : context_(context) {}

OperationResult AdministrationService::set_maximum_credits(const UserIdentity& user,
                                                           int maximum_credits) {
    // 1. 角色
    if (!PermissionRules::can_manage_catalog(user)) {
        return {ResultCode::PermissionDenied, "Only administrators can change the credit cap"};
    }
    // 2. 输入
    if (maximum_credits <= 0 || maximum_credits > 60) {
        return {ResultCode::InvalidInput, "Maximum credits must be within 1..60"};
    }
    // 3. 业务前置：本轮已经开放过，上限就被锁住
    if (context_.observe_registration_round()) {
        return {ResultCode::PermissionDenied,
                "Registration round already started, the credit cap is locked"};
    }

    RegistrationRule rule = context_.rule();   // 取副本，不直接改内部字段
    rule.maximum_credits = maximum_credits;
    context_.apply_rule(rule);                 // 校验全通过后才提交
    return {ResultCode::Success, "Maximum credits updated"};
}

OperationResult AdministrationService::set_registration_period(const UserIdentity& user,
                                                               const std::string& opens_at,
                                                               const std::string& closes_at) {
    if (!PermissionRules::can_set_registration_period(user)) {
        return {ResultCode::PermissionDenied, "Only administrators can set the period"};
    }

    long long open_key = 0;
    long long close_key = 0;
    if (!parse_iso_key(opens_at, open_key) || !parse_iso_key(closes_at, close_key)) {
        return {ResultCode::InvalidInput, "Time format must be YYYY-MM-DDTHH:MM"};
    }
    if (open_key >= close_key) {
        return {ResultCode::InvalidInput, "The opening time must be earlier than closing"};
    }

    RegistrationRule rule = context_.rule();
    rule.opens_at = opens_at;
    rule.closes_at = closes_at;
    rule.registration_open = true;
    context_.apply_rule(rule);
    return {ResultCode::Success, "Registration period updated"};
}

OperationResult AdministrationService::close_registration(const UserIdentity& user) {
    if (!PermissionRules::can_set_registration_period(user)) {
        return {ResultCode::PermissionDenied, "Only administrators can close registration"};
    }
    RegistrationRule rule = context_.rule();
    rule.registration_open = false;
    // 手动关闭时清空起止时间：否则普通刷新会按时间把它重新打开
    rule.opens_at.clear();
    rule.closes_at.clear();
    context_.apply_rule(rule);
    return {ResultCode::Success, "Registration closed"};
}

RegistrationRoundStatus AdministrationService::current_round() const {
    RegistrationRoundStatus round{};
    round.opened = context_.round_opened();
    const RegistrationRule rule = context_.rule();
    round.opens_at = rule.opens_at;
    round.closes_at = rule.closes_at;
    return round;
}

}  // namespace course_selection
