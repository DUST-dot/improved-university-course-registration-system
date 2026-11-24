#pragma once
// 2.23 US-23 Maintainability and 2.24 US-24 Release and buffer.
//
// The maintainability claim is that a newcomer can build and verify the project
// from the README alone. The release claim is that one command proves the build is
// shippable. Both are tested rather than asserted in prose.
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct ReleaseReport {
    int checks{0};
    int failures{0};
    std::vector<std::string> details;
};

class ReleaseCheck {
public:
    explicit ReleaseCheck(const SystemContext& context) : context_(context) {}

    // Runs every invariant the project cares about and returns a non-zero exit
    // code when anything is off, so CI can gate on it.
    ReleaseReport run() const;

private:
    const SystemContext& context_;
};

}  // namespace course_selection
