#include "usability_service.hpp"

#include <cctype>

namespace course_selection {

std::vector<FieldProblem> UsabilityService::validate_login(const std::string& account,
                                                           const std::string& password) {
    std::vector<FieldProblem> problems;
    if (account.size() < 4 || account.size() > 20) {
        problems.push_back({"account", "Account must be 4 to 20 characters"});
    }
    for (char ch : account) {
        if (!std::isalnum(static_cast<unsigned char>(ch))) {
            problems.push_back({"account", "Account may only contain letters and digits"});
            break;
        }
    }
    if (password.empty()) problems.push_back({"password", "Password must not be empty"});
    return problems;
}

std::vector<FieldProblem> UsabilityService::validate_enroll_form(const std::string& class_id) {
    std::vector<FieldProblem> problems;
    if (class_id.empty()) {
        problems.push_back({"class_id", "Please choose a teaching class"});
        return problems;
    }
    for (char ch : class_id) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            problems.push_back({"class_id", "Class id must be a number"});
            return problems;
        }
    }
    if (class_id.size() > 9) problems.push_back({"class_id", "Class id is too long"});
    return problems;
}

std::string UsabilityService::hint_for(ResultCode code) {
    switch (code) {
        case ResultCode::TeachingClassFull:
            return "Pick another class, the list shows how many seats are left";
        case ResultCode::AlreadyEnrolled:
            return "You already have this course, drop it first if you want to switch";
        case ResultCode::TimeConflict:
            return "Check the conflicting course, then choose a different class";
        case ResultCode::CreditLimitExceeded:
            return "Drop a course or pick a lighter one";
        case ResultCode::RegistrationClosed:
            return "Registration is closed, watch the notice for the next round";
        default:
            return "Something went wrong, please try again";
    }
}

}  // namespace course_selection
