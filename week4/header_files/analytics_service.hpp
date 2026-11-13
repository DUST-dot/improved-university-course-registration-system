#pragma once
// 2.16 US-18 Enrollment distribution chart.
// A pure aggregation over Active records, grouped by one dimension at a time.
#include <string>
#include <utility>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

class AnalyticsService {
public:
    explicit AnalyticsService(const SystemContext& context) : context_(context) {}

    // dimension: "course" groups by course code, "college" by the teacher department.
    // Any other value is InvalidInput rather than an empty chart.
    OperationResult distribution(const std::string& dimension,
                                 std::vector<std::pair<std::string, int>>& result) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
