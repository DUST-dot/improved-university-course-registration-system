// 2.12 US-15 import course file

#include "test_support_week4.hpp"

#include <sstream>

using namespace course_selection;
using test_support::check;

int main() {
    ImportService service;

    // Happy path: three good rows.
    {
        std::istringstream csv("code,name,credits\nSE201,Algorithms,4\nSE202,Networks,3\nSE203,Compilers,5\n");
        const ImportResult result = service.import_courses_csv(csv, {});
        check(result.total_lines == 3, "three data rows counted");
        check(result.accepted.size() == 3, "three rows accepted");
        check(result.errors.empty(), "no errors on a clean file");
        check(result.accepted[0].course_code == "SE201", "first code parsed");
        check(result.accepted[0].credits == 4.0, "credits parsed");
    }

    // Blank lines are skipped, not reported.
    {
        std::istringstream csv("code,name,credits\nSE201,Algorithms,4\n\n\nSE202,Networks,3\n");
        const ImportResult result = service.import_courses_csv(csv, {});
        check(result.total_lines == 2, "blank lines are not counted as data");
        check(result.accepted.size() == 2, "both real rows accepted");
    }

    // Every rejection reason is reported with its line number.
    {
        std::istringstream csv(
            "code,name,credits\n"
            "1BAD,Bad code,4\n"       // line 2: code must start with a letter
            "SE301,,3\n"              // line 3: empty name
            "SE302,Numeric,x\n"       // line 4: credits not a number
            "SE303,Out of range,99\n" // line 5: credits out of range
            "SE304,Too few\n"         // line 6: missing column
            "SE305,Good,3\n");        // line 7: accepted
        const ImportResult result = service.import_courses_csv(csv, {});
        check(result.total_lines == 6, "six data rows counted");
        check(result.accepted.size() == 1, "only the valid row is accepted");
        check(result.errors.size() == 5, "five rows rejected");
        check(result.errors[0].kind == ImportError::BadCodeFormat, "bad code detected");
        check(result.errors[0].line_no == 2, "line number is 1-based and accurate");
        check(result.errors[1].kind == ImportError::EmptyField, "empty field detected");
        check(result.errors[2].kind == ImportError::NonNumericCredits, "non numeric credits detected");
        check(result.errors[3].kind == ImportError::CreditsOutOfRange, "range violation detected");
        check(result.errors[4].kind == ImportError::MissingColumn, "short row detected");
        check(result.errors[1].raw_line.find("SE301") != std::string::npos,
              "raw line is kept for review");
    }

    // Duplicates inside one file are rejected, never merged.
    {
        std::istringstream csv("code,name,credits\nSE401,One,3\nSE401,Two,4\n");
        const ImportResult result = service.import_courses_csv(csv, {});
        check(result.accepted.size() == 1, "duplicate within the file is skipped");
        check(result.errors.size() == 1, "duplicate is reported");
        check(result.errors[0].kind == ImportError::DuplicateIdentifier, "duplicate reason reported");
    }

    // Duplicates against the existing catalogue are rejected too.
    {
        std::istringstream csv("code,name,credits\nSE101,Existing,3\n");
        const ImportResult result = service.import_courses_csv(csv, {"SE101"});
        check(result.accepted.empty(), "existing code is not re-imported");
        check(result.errors.size() == 1, "existing code reported");
        check(result.errors[0].kind == ImportError::DuplicateIdentifier, "reported as duplicate");
    }

    // Empty file and header-only file are not errors.
    {
        std::istringstream empty("");
        check(service.import_courses_csv(empty, {}).errors.empty(), "empty file reports nothing");
        std::istringstream header_only("code,name,credits\n");
        const ImportResult result = service.import_courses_csv(header_only, {});
        check(result.total_lines == 0 && result.accepted.empty(), "header only yields nothing");
    }

    // A header with too few columns stops the import immediately.
    {
        std::istringstream csv("code\nSE501,One,3\n");
        const ImportResult result = service.import_courses_csv(csv, {});
        check(result.accepted.empty(), "bad header rejects the file");
        check(result.errors.size() == 1 && result.errors[0].kind == ImportError::MissingColumn,
              "bad header reported as MissingColumn");
    }

    std::cout << "  import_error_name(DuplicateIdentifier) = "
              << import_error_name(ImportError::DuplicateIdentifier) << '\n';
    return test_support::finish();
}
