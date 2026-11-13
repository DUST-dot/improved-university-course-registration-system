#include "feedback_service.hpp"

namespace course_selection {

std::string FeedbackService::describe(const OperationResult& result) {
    switch (result.code) {
        case ResultCode::Success:                return result.message;
        case ResultCode::RegistrationClosed:     return "Registration is closed now.";
        case ResultCode::AlreadyEnrolled:        return "You have already enrolled in this course.";
        case ResultCode::TimeConflict:           return result.message;
        case ResultCode::CreditLimitExceeded:    return "This would exceed your credit limit.";
        case ResultCode::TeachingClassFull:      return "This class is full, please pick another one.";
        case ResultCode::TeachingClassNotFound:  return "That teaching class does not exist.";
        case ResultCode::PermissionDenied:       return "You do not have permission for this action.";
        case ResultCode::InvalidInput:           return "Invalid input: " + result.message;
        default:                                 return "Operation failed: " + result.message;
    }
}

}  // namespace course_selection
