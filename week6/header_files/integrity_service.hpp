#pragma once
// 2.20 US-20 Data reliability.
// Three invariants that must hold at all times: no duplicate active record, no
// enrollment pointing at a missing student or class, and no class over capacity.
// The check returns every problem at once instead of stopping at the first one,
// because a repair pass needs the whole list.
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

class IntegrityService {
public:
    explicit IntegrityService(const SystemContext& context) : context_(context) {}

    // Returns Success when the list comes back empty.
    OperationResult check(std::vector<std::string>& problems) const;

    // Rebuilds the seat counter from the enrollment records.
    void resync();

private:
    const SystemContext& context_;
};

}  // namespace course_selection
