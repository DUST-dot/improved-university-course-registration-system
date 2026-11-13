#pragma once
// 2.15 US-05 Course selection result feedback.
// One place turns a result code into a sentence the student can act on, so the
// UI never prints a raw code and never maintains its own copy of the mapping.
#include <string>

#include "domain.hpp"

namespace course_selection {

class FeedbackService {
public:
    static std::string describe(const OperationResult& result);
};

}  // namespace course_selection
