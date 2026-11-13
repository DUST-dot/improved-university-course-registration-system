#pragma once

// ============================================================================
// US-01 section
//
//
// ============================================================================

#include <map>
#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct AccountCredential {
    int user_id;
    std::string account_number;
    Role role;
    std::string salt;
    std::string password_hash;
};

struct SessionInfo {
    std::string token;
    UserIdentity user;
    std::string issued_at;
    std::string expires_at;
};

class PasswordHasher {
public:
    static constexpr int kIterations = 1000;

    static std::string sha256_hex(const std::string& text);
    static std::string make_salt(const std::string& seed);
    static std::string hash(const std::string& password, const std::string& salt);
    static bool verify(const std::string& password, const std::string& salt,
                       const std::string& expected_hash);
    static bool acceptable_password(const std::string& password);
};

struct LoginPolicy {
    int max_failures{5};
    int lock_minutes{15};
    int session_minutes{120};
};

class AuthenticationService {
public:
    AuthenticationService(SystemContext& context,
                          std::vector<AccountCredential> credentials,
                          LoginPolicy policy = LoginPolicy{});

    OperationResult login(const std::string& account_number,
                          const std::string& password,
                          UserIdentity& identity,
                          std::string& token);

    OperationResult validate_session(const std::string& token, UserIdentity& identity);
    OperationResult logout(const std::string& token);

    bool is_locked(const std::string& account_number) const;
    int failure_count(const std::string& account_number) const;

    static OperationResult require_role(const UserIdentity& user, Role role);

    static bool account_number_well_formed(const std::string& account_number);

private:
    struct AccountState {
        int failures{0};
        std::string locked_until;
    };

    const AccountCredential* find_credential(const std::string& account_number) const;
    bool role_account_active(const AccountCredential& credential) const;
    std::string next_token(const std::string& account_number);
    static AccountCredential make_credential(int user_id, const std::string& number,
                                             Role role, const std::string& password);

public:
    static AccountCredential credential_for(int user_id, const std::string& number,
                                            Role role, const std::string& password) {
        return make_credential(user_id, number, role, password);
    }

private:
    SystemContext& context_;
    std::vector<AccountCredential> credentials_;
    LoginPolicy policy_;
    std::map<std::string, AccountState> states_;
    std::map<std::string, SessionInfo> sessions_;
    int token_counter_{0};
};

}  // namespace course_selection
