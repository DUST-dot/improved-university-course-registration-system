#include "import_service.hpp"

#include <cctype>
#include <set>
#include <sstream>

namespace course_selection {
namespace {

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(begin, end - begin);
}

// Plain comma split. Fields containing commas are out of scope for this format.
std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream stream(line);
    while (std::getline(stream, field, ',')) fields.push_back(field);
    if (!line.empty() && line.back() == ',') fields.emplace_back();   // trailing empty field
    return fields;
}

// Course code: starts with a letter, letters and digits only, 2..16 characters.
bool valid_code(const std::string& code) {
    if (code.size() < 2 || code.size() > 16) return false;
    if (!std::isalpha(static_cast<unsigned char>(code[0]))) return false;
    for (char ch : code) {
        if (!std::isalnum(static_cast<unsigned char>(ch))) return false;
    }
    return true;
}

bool parse_int(const std::string& text, int& value) {
    if (text.empty()) return false;
    std::size_t index = 0;
    if (text[0] == '+' || text[0] == '-') index = 1;
    if (index >= text.size()) return false;
    for (std::size_t i = index; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
    }
    try {
        value = std::stoi(text);
    } catch (...) {
        return false;
    }
    return true;
}

}  // namespace

const char* import_error_name(ImportError kind) {
    switch (kind) {
        case ImportError::MissingColumn:       return "MissingColumn";
        case ImportError::EmptyField:          return "EmptyField";
        case ImportError::BadCodeFormat:       return "BadCodeFormat";
        case ImportError::CreditsOutOfRange:   return "CreditsOutOfRange";
        case ImportError::NonNumericCredits:   return "NonNumericCredits";
        case ImportError::DuplicateIdentifier: return "DuplicateIdentifier";
    }
    return "Unknown";
}

ImportResult ImportService::import_courses_csv(
    std::istream& input, const std::vector<std::string>& existing_codes) const {
    ImportResult result;

    std::string line;
    if (!std::getline(input, line)) return result;   // empty file

    // Strip a UTF-8 BOM, otherwise the first header cell never matches.
    if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF) {
        line.erase(0, 3);
    }

    const std::vector<std::string> header = split_csv(line);
    if (header.size() < 3) {
        result.errors.push_back({1, line, ImportError::MissingColumn,
                                 "The header needs at least three columns: code,name,credits"});
        return result;
    }

    std::set<std::string> seen(existing_codes.begin(), existing_codes.end());
    std::size_t line_no = 1;

    while (std::getline(input, line)) {
        ++line_no;
        if (trim(line).empty()) continue;   // blank line is not an error
        ++result.total_lines;

        const std::vector<std::string> fields = split_csv(line);
        if (fields.size() < 3) {
            result.errors.push_back(
                {line_no, line, ImportError::MissingColumn, "Expected three columns"});
            continue;
        }

        const std::string code = trim(fields[0]);
        const std::string name = trim(fields[1]);
        const std::string credits_text = trim(fields[2]);

        if (code.empty() || name.empty() || credits_text.empty()) {
            result.errors.push_back({line_no, line, ImportError::EmptyField,
                                     "Code, name and credits are all required"});
            continue;
        }
        if (!valid_code(code)) {
            result.errors.push_back({line_no, line, ImportError::BadCodeFormat,
                                     "Code must start with a letter and use 2..16 alphanumerics"});
            continue;
        }
        int credits = 0;
        if (!parse_int(credits_text, credits)) {
            result.errors.push_back(
                {line_no, line, ImportError::NonNumericCredits, "Credits must be an integer"});
            continue;
        }
        if (credits < 1 || credits > 10) {
            result.errors.push_back({line_no, line, ImportError::CreditsOutOfRange,
                                     "Credits must be within 1..10"});
            continue;
        }
        if (!seen.insert(code).second) {
            // Duplicates are skipped, never overwritten: an import must not silently
            // change an existing course.
            result.errors.push_back({line_no, line, ImportError::DuplicateIdentifier,
                                     "Course code already exists"});
            continue;
        }

        Course course{};
        course.course_id = static_cast<int>(result.accepted.size()) + 1;   // caller reassigns
        course.course_code = code;
        course.course_name = name;
        course.credits = credits;
        result.accepted.push_back(course);
    }

    return result;
}

}  // namespace course_selection
