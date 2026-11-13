#pragma once
// 2.18 US-08 Course recommendation.
// Two layers: a rule score that always works, plus an optional collaborative
// bonus that may be unavailable. The rule layer is why this story can be cut
// first when time runs short: it degrades to something explainable.
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct Recommendation {
    int teaching_class_id{0};
    std::string course_code;
    int score{0};
    std::string reason;
};

class RecommendationService {
public:
    explicit RecommendationService(const SystemContext& context) : context_(context) {}

    OperationResult recommend(int student_id,
                              std::size_t limit,
                              std::vector<Recommendation>& result) const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
