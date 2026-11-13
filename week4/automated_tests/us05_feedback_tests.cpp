// 2.15 US-05 course selection result feedback
#include "test_support_week5.hpp"

using namespace course_selection;
using test_support::check;

int main() {
    check(FeedbackService::describe({ResultCode::Success, "Enrolled"}) == "Enrolled",
          "success passes the message through");
    check(FeedbackService::describe({ResultCode::TeachingClassFull, ""}).find("full") !=
              std::string::npos, "full class explains itself");
    check(FeedbackService::describe({ResultCode::AlreadyEnrolled, ""}).find("already") !=
              std::string::npos, "duplicate explains itself");
    check(FeedbackService::describe({ResultCode::RegistrationClosed, ""}).find("closed") !=
              std::string::npos, "closed window explains itself");
    check(FeedbackService::describe({ResultCode::CreditLimitExceeded, ""}).find("credit") !=
              std::string::npos, "credit cap explains itself");
    check(FeedbackService::describe({ResultCode::PermissionDenied, ""}).find("permission") !=
              std::string::npos, "permission explains itself");
    check(FeedbackService::describe({ResultCode::InvalidInput, "bad code"}).find("bad code") !=
              std::string::npos, "invalid input keeps the detail");
    check(FeedbackService::describe({ResultCode::TimeConflict, "clash detail"}).find("clash") !=
              std::string::npos, "time conflict keeps the detail");
    // Every branch returns something, never an empty string.
    check(!FeedbackService::describe({ResultCode::RecordNotFound, ""}).empty(),
          "unknown codes still produce a sentence");
    return test_support::finish();
}
