#pragma once

// ============================================================================
// US-13 section
//
//
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct RegistrationRoundStatus {
    bool opened{false};
    std::string opens_at{};
    std::string closes_at{};
};

class AdministrationService {
public:
    explicit AdministrationService(SystemContext& context);

    OperationResult set_maximum_credits(const UserIdentity& user, int maximum_credits);

    OperationResult set_registration_period(const UserIdentity& user,
                                            const std::string& opens_at,
                                            const std::string& closes_at);

    OperationResult close_registration(const UserIdentity& user);

    RegistrationRoundStatus current_round() const;

private:
    SystemContext& context_;
};

}  // namespace course_selection
