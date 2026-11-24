#pragma once

// ISO timestamp helpers. Login and the registration window share one parser so the
// format cannot drift between them.
//
// Format is fixed width YYYY-MM-DDTHH:MM; values are converted to minutes since
// 1970 before comparing or adding, because string ordering is not enough for
// "fifteen minutes later".

#include <string>

namespace course_selection {

// Returns false without touching minute when the text is malformed.
bool parse_iso_minute(const std::string& text, long long& minute);

// True when a is earlier than b. Falls back to string comparison if either fails.
bool moment_before(const std::string& a, const std::string& b);

// Adds minutes; returns the input unchanged if it cannot be parsed.
std::string add_minutes(const std::string& moment, int minutes);

std::string format_iso_minute(long long minute);

// Local time as YYYY-MM-DDTHH:MM.
std::string current_timestamp();

}  // namespace course_selection
