#pragma once
// 2.22 US-22 Usability.
// Every failure must come back as a sentence a student can act on, and every form
// field must be validated before it reaches the service layer.
#include <string>
#include <vector>

#include "domain.hpp"

namespace course_selection {

struct FieldProblem {
    std::string field;
    std::string message;
};

class UsabilityService {
public:
    // Validates the login form before it is submitted.
    static std::vector<FieldProblem> validate_login(const std::string& account,
                                                    const std::string& password);

    // Validates the enrollment form (a class id must be a positive number).
    static std::vector<FieldProblem> validate_enroll_form(const std::string& class_id);

    // One sentence per failure, ranked so the first one is the one to fix.
    static std::string hint_for(ResultCode code);
};

}  // namespace course_selection
