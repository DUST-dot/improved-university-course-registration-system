#pragma once

// ============================================================================
// 2.12 US-15 Import course file
//
// Adapted from the standalone `import` programme. The rule that matters: one bad
// row must not destroy the whole batch, and it must not be silently dropped
// either. Every rejected row is reported with its line number, the raw text and a
// machine-readable reason, so the administrator can fix the file and re-upload.
//
// Accepted rows are returned, not written: the caller decides when to commit them.
// ============================================================================

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

#include "domain.hpp"

namespace course_selection {

enum class ImportError {
    MissingColumn,
    EmptyField,
    BadCodeFormat,
    CreditsOutOfRange,
    NonNumericCredits,
    DuplicateIdentifier
};

const char* import_error_name(ImportError kind);

struct RowError {
    std::size_t line_no{0};   // 1-based, header is line 1
    std::string raw_line;
    ImportError kind{ImportError::EmptyField};
    std::string reason;
};

struct ImportResult {
    std::vector<Course> accepted;
    std::vector<RowError> errors;
    std::size_t total_lines{0};   // data rows, header excluded
};

class ImportService {
public:
    // existing_codes: course codes already in the catalogue, used for cross-batch
    // de-duplication. The stream is consumed from the header line onwards.
    ImportResult import_courses_csv(std::istream& input,
                                    const std::vector<std::string>& existing_codes) const;
};

}  // namespace course_selection
