#pragma once

// ============================================================================
// US-03 section
//
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct SeatsView {
    int teaching_class_id{0};
    int course_id{0};
    std::string course_code;
    std::string course_name;
    int capacity{0};
    int enrollment_count{0};
    int remaining_seats{0};
};

class StudentQueryService {
public:
    explicit StudentQueryService(const SystemContext& context) : context_(context) {}

    OperationResult view_available_seats(int teaching_class_id, SeatsView& result) const;

    OperationResult list_available_seats(std::vector<SeatsView>& result) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
