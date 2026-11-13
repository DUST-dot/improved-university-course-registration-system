#pragma once

// ============================================================================
// 登录与身份（US-01 学生 / US-09 教师 / US-13 教务）
//
// 只做一件事：把「账号 + 密码」换成业务模块要的 UserIdentity。
// 不读课程、不改选课记录；账号资料在 SystemContext 里，这里只引用。
//
// 三条规矩：
//   1. 密码不存明文，只存「盐 + 迭代哈希」，校验时重算再比对。
//   2. 账号不存在与密码错误返回同一个失败原因，防止用登录接口枚举学号。
//   3. 连续失败达上限后锁定；锁定期内即使密码正确也拒绝；到期按时间自动解锁。
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
    std::string salt;           // 十六进制
    std::string password_hash;  // 十六进制
};

struct SessionInfo {
    std::string token;
    UserIdentity user;
    std::string issued_at;
    std::string expires_at;
};

// 只用标准库实现：SHA-256(FIPS 180-4) + 盐 + 迭代，避免引入 OpenSSL。
class PasswordHasher {
public:
    static constexpr int kIterations = 1000;

    static std::string sha256_hex(const std::string& text);
    static std::string make_salt(const std::string& seed);
    static std::string hash(const std::string& password, const std::string& salt);
    static bool verify(const std::string& password, const std::string& salt,
                       const std::string& expected_hash);
    // 设置密码时的格式要求；登录时**不**调用它，否则用错误信息就能反推长度。
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

    // 各端入口的第一句：拿学生账号进教师端会在这里被挡住。
    static OperationResult require_role(const UserIdentity& user, Role role);

    static bool account_number_well_formed(const std::string& account_number);

private:
    struct AccountState {
        int failures{0};
        std::string locked_until;  // 空表示未锁定
    };

    const AccountCredential* find_credential(const std::string& account_number) const;
    bool role_account_active(const AccountCredential& credential) const;
    std::string next_token(const std::string& account_number);
    // 供测试与演示直接造凭据，不必在外部重复写盐和哈希。
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
