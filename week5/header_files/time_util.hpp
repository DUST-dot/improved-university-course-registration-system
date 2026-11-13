#pragma once

// ============================================================================
// 时间工具（统一出口）
//
// 原来两套代码各有一份：week1 的在 authentication_service.cpp 的匿名命名空间里，
// 2.5–2.9 的在 registration_window.cpp 里以自由函数暴露。合并后集中到这里，
// 避免同一套 ISO 解析在三个地方各写一遍。
//
// 格式统一为定长 YYYY-MM-DDTHH:MM；解析成"自 1970-01-01 起的分钟数"再比较与加减，
// 因为定长格式虽然字典序等于时间序，但"加 15 分钟""加 2 小时"必须走真正的算术。
// ============================================================================

#include <string>

namespace course_selection {

// 解析失败返回 false，且不修改 minute。
bool parse_iso_minute(const std::string& text, long long& minute);

// a 是否早于 b。两边都解析不了时退回字符串比较。
bool moment_before(const std::string& a, const std::string& b);

// 在给定时刻上加分钟；解析失败时原样返回。
std::string add_minutes(const std::string& moment, int minutes);

// 分钟数转回定长字符串。
std::string format_iso_minute(long long minute);

// 当前本地时间，格式 YYYY-MM-DDTHH:MM。
std::string current_timestamp();

}  // namespace course_selection
