#include "release_check.hpp"

#include "integrity_service.hpp"

namespace course_selection {
namespace {

void expect(ReleaseReport& report, bool condition, const std::string& what) {
    ++report.checks;
    if (!condition) {
        ++report.failures;
        report.details.push_back("FAILED: " + what);
    }
}

}  // namespace

ReleaseReport ReleaseCheck::run() const {
    ReleaseReport report;

    // 1. Catalogue sanity: every class belongs to a real course.
    for (const TeachingClass& item : context_.teaching_classes()) {
        Course course{};
        expect(report, context_.find_course(item.course_id, course),
               "class " + std::to_string(item.teaching_class_id) + " has a course");
        expect(report, item.capacity > 0, "class " + std::to_string(item.teaching_class_id) +
                                              " has a positive capacity");
    }

    // 2. Data integrity.
    IntegrityService integrity(context_);
    std::vector<std::string> problems;
    integrity.check(problems);
    expect(report, problems.empty(), "data integrity");
    for (const std::string& problem : problems) report.details.push_back("  " + problem);

    // 3. Seat arithmetic: count plus remaining must equal capacity everywhere.
    for (const TeachingClass& item : context_.teaching_classes()) {
        const int active = context_.enrollment_count(item.teaching_class_id);
        const int seats = context_.remaining_seats(item.teaching_class_id);
        expect(report, active + seats == item.capacity,
               "seats add up for class " + std::to_string(item.teaching_class_id));
        expect(report, seats >= 0, "no negative seats for class " +
                                       std::to_string(item.teaching_class_id));
    }

    return report;
}

}  // namespace course_selection
