#include "time_util.hpp"

#include <cstdio>
#include <ctime>

namespace course_selection {
namespace {

bool is_digit(char ch) { return ch >= '0' && ch <= '9'; }

int read_digits(const std::string& text, std::size_t pos, std::size_t count) {
    int value = 0;
    for (std::size_t i = 0; i < count; ++i) {
        const char ch = text[pos + i];
        if (!is_digit(ch)) return -1;
        value = value * 10 + (ch - '0');
    }
    return value;
}

bool leap_year(int year) { return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0; }

int days_in_month(int year, int month) {
    static const int table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && leap_year(year)) return 29;
    return table[month - 1];
}

// 公历日期转"自 1970-01-01 起的天数"（Howard Hinnant 算法）
long long days_from_civil(int year, unsigned month, unsigned day) {
    year -= month <= 2 ? 1 : 0;
    const long long era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);
    const unsigned doy = (153u * (month + (month > 2 ? -3 : 9)) + 2u) / 5u + day - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return era * 146097LL + static_cast<long long>(doe) - 719468LL;
}

void civil_from_days(long long z, int& year, unsigned& month, unsigned& day) {
    z += 719468LL;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460u + doe / 36524u - doe / 146096u) / 365u;
    const unsigned doy = doe - (365u * yoe + yoe / 4u - yoe / 100u);
    const unsigned mp = (5u * doy + 2u) / 153u;
    day = doy - (153u * mp + 2u) / 5u + 1u;
    month = mp + (mp < 10 ? 3u : static_cast<unsigned>(-9));
    year = static_cast<int>(yoe + era * 400 + (month <= 2 ? 1 : 0));
}

}  // namespace

bool parse_iso_minute(const std::string& text, long long& minute) {
    if (text.size() != 16) return false;
    if (text[4] != '-' || text[7] != '-' || text[10] != 'T' || text[13] != ':') return false;

    const int year = read_digits(text, 0, 4);
    const int month = read_digits(text, 5, 2);
    const int day = read_digits(text, 8, 2);
    const int hour = read_digits(text, 11, 2);
    const int mins = read_digits(text, 14, 2);

    if (year < 0 || month < 1 || month > 12) return false;
    if (day < 1 || day > days_in_month(year, month)) return false;
    if (hour < 0 || hour > 23) return false;
    if (mins < 0 || mins > 59) return false;

    minute = days_from_civil(year, static_cast<unsigned>(month), static_cast<unsigned>(day)) *
                 1440LL +
             static_cast<long long>(hour) * 60LL + static_cast<long long>(mins);
    return true;
}

bool moment_before(const std::string& a, const std::string& b) {
    long long left = 0;
    long long right = 0;
    if (!parse_iso_minute(a, left) || !parse_iso_minute(b, right)) {
        return a < b;   // 定长格式下字典序等价于时间序
    }
    return left < right;
}

std::string format_iso_minute(long long minute) {
    long long days = minute / 1440LL;
    long long rest = minute % 1440LL;
    if (rest < 0) {
        rest += 1440LL;
        --days;
    }
    int year = 0;
    unsigned month = 0;
    unsigned day = 0;
    civil_from_days(days, year, month, day);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02u-%02uT%02lld:%02lld", year, month, day,
                  rest / 60LL, rest % 60LL);
    return std::string(buffer);
}

std::string add_minutes(const std::string& moment, int minutes) {
    long long base = 0;
    if (!parse_iso_minute(moment, base)) return moment;
    return format_iso_minute(base + static_cast<long long>(minutes));
}

std::string current_timestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[32] = {0};
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d", local.tm_year + 1900,
                  local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min);
    return std::string(buffer);
}

}  // namespace course_selection
