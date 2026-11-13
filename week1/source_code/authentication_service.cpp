#include "authentication_service.hpp"

#include <cctype>
#include <cstdio>

namespace course_selection {
namespace {

// ISO 时刻（YYYY-MM-DDTHH:MM）折成分钟数，才能做“加 15 分钟”和比较。
long long days_from_civil(int year, unsigned month, unsigned day) {
    year -= month <= 2 ? 1 : 0;
    const long long era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);
    const unsigned doy = (153u * (month + (month > 2 ? -3 : 9)) + 2u) / 5u + day - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return era * 146097LL + static_cast<long long>(doe) - 719468LL;
}

void civil_from_days(long long z, int& year, unsigned& month, unsigned& day) {
    z += 719468LL;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460u + doe / 36524u - doe / 146096u) / 365u;
    const unsigned doy = doe - (365u * yoe + yoe / 4u - yoe / 100u);
    const unsigned mp = (5u * doy + 2u) / 153u;
    day = doy - (153u * mp + 2u) / 5u + 1u;
    month = mp + (mp < 10 ? 3u : static_cast<unsigned>(-9));
    year = static_cast<int>(yoe + era * 400 + (month <= 2 ? 1 : 0));
}

bool parse_iso_minute(const std::string& text, long long& minutes) {
    int year = 0;
    unsigned month = 0, day = 0, hour = 0, minute = 0;
    if (std::sscanf(text.c_str(), "%4d-%2u-%2uT%2u:%2u", &year, &month, &day,
                    &hour, &minute) != 5) {
        return false;
    }
    if (month == 0 || month > 12 || day == 0 || day > 31 || hour > 23 || minute > 59) {
        return false;
    }
    minutes = days_from_civil(year, month, day) * 1440LL +
              static_cast<long long>(hour) * 60LL + static_cast<long long>(minute);
    return true;
}

std::string format_iso_minute(long long minutes) {
    long long days = minutes / 1440LL;
    long long rest = minutes % 1440LL;
    if (rest < 0) {
        rest += 1440LL;
        --days;
    }
    int year = 0;
    unsigned month = 0, day = 0;
    civil_from_days(days, year, month, day);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02u-%02uT%02lld:%02lld", year, month, day,
                  rest / 60LL, rest % 60LL);
    return std::string(buffer);
}

std::string add_minutes(const std::string& moment, int minutes) {
    long long base = 0;
    if (!parse_iso_minute(moment, base)) return moment;
    return format_iso_minute(base + static_cast<long long>(minutes));
}

bool moment_before(const std::string& a, const std::string& b) {
    long long left = 0, right = 0;
    if (parse_iso_minute(a, left) && parse_iso_minute(b, right)) return left < right;
    return a < b;
}

std::string to_lower_copy(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

unsigned int rotate_right(unsigned int value, unsigned int bits) {
    return (value >> bits) | (value << (32u - bits));
}

std::string sha256_hex_impl(const std::string& message) {
    static const unsigned int kRound[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u,
        0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
        0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u,
        0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
        0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
        0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
        0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au,
        0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
        0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

    unsigned int hash[8] = {0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                            0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};

    std::vector<unsigned char> data(message.begin(), message.end());
    const unsigned long long bit_length = static_cast<unsigned long long>(data.size()) * 8ULL;
    data.push_back(0x80u);
    while (data.size() % 64u != 56u) data.push_back(0x00u);
    for (int shift = 56; shift >= 0; shift -= 8) {
        data.push_back(static_cast<unsigned char>((bit_length >> shift) & 0xFFULL));
    }

    for (std::size_t offset = 0; offset < data.size(); offset += 64u) {
        unsigned int w[64] = {0};
        for (int i = 0; i < 16; ++i) {
            const std::size_t base = offset + static_cast<std::size_t>(i) * 4u;
            w[i] = (static_cast<unsigned int>(data[base]) << 24) |
                   (static_cast<unsigned int>(data[base + 1]) << 16) |
                   (static_cast<unsigned int>(data[base + 2]) << 8) |
                   static_cast<unsigned int>(data[base + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            const unsigned int s0 = rotate_right(w[i - 15], 7) ^
                                    rotate_right(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const unsigned int s1 = rotate_right(w[i - 2], 17) ^
                                    rotate_right(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        unsigned int a = hash[0], b = hash[1], c = hash[2], d = hash[3];
        unsigned int e = hash[4], f = hash[5], g = hash[6], h = hash[7];
        for (int i = 0; i < 64; ++i) {
            const unsigned int s1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^ rotate_right(e, 25);
            const unsigned int choice = (e & f) ^ ((~e) & g);
            const unsigned int temp1 = h + s1 + choice + kRound[i] + w[i];
            const unsigned int s0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^ rotate_right(a, 22);
            const unsigned int majority = (a & b) ^ (a & c) ^ (b & c);
            const unsigned int temp2 = s0 + majority;
            h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
        }
        hash[0] += a; hash[1] += b; hash[2] += c; hash[3] += d;
        hash[4] += e; hash[5] += f; hash[6] += g; hash[7] += h;
    }

    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(64);
    for (unsigned int word : hash) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            out.push_back(digits[(word >> shift) & 0x0Fu]);
        }
    }
    return out;
}

}  // namespace

// ---------------------------------------------------------------------------
// PasswordHasher
// ---------------------------------------------------------------------------

std::string PasswordHasher::sha256_hex(const std::string& text) {
    return sha256_hex_impl(text);
}

std::string PasswordHasher::make_salt(const std::string& seed) {
    // 确定性盐：同一份演示数据每次构造结果相同，测试才能断言固定哈希。
    // 真实系统应换成随机源，接口不变。
    return sha256_hex_impl("course_selection|salt|" + seed).substr(0, 16);
}

std::string PasswordHasher::hash(const std::string& password, const std::string& salt) {
    std::string current = sha256_hex_impl(salt + ":" + password);
    for (int i = 1; i < kIterations; ++i) {
        current = sha256_hex_impl(current + password);
    }
    return current;
}

bool PasswordHasher::verify(const std::string& password, const std::string& salt,
                            const std::string& expected_hash) {
    const std::string actual = hash(password, salt);
    if (actual.size() != expected_hash.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < actual.size(); ++i) {
        diff |= static_cast<unsigned char>(actual[i] ^ expected_hash[i]);
    }
    return diff == 0;
}

bool PasswordHasher::acceptable_password(const std::string& password) {
    if (password.size() < 8 || password.size() > 20) return false;
    bool has_letter = false;
    bool has_digit = false;
    for (char ch : password) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalpha(uch) != 0) {
            has_letter = true;
        } else if (std::isdigit(uch) != 0) {
            has_digit = true;
        } else if (uch == ' ' || uch == '\t') {
            return false;
        }
    }
    return has_letter && has_digit;
}

// ---------------------------------------------------------------------------
// AuthenticationService
// ---------------------------------------------------------------------------

AuthenticationService::AuthenticationService(SystemContext& context,
                                             std::vector<AccountCredential> credentials,
                                             LoginPolicy policy)
    : context_(context), credentials_(std::move(credentials)), policy_(policy) {}

AccountCredential AuthenticationService::make_credential(int user_id,
                                                         const std::string& number,
                                                         Role role,
                                                         const std::string& password) {
    AccountCredential credential;
    credential.user_id = user_id;
    credential.account_number = number;
    credential.role = role;
    credential.salt = PasswordHasher::make_salt(number);
    credential.password_hash = PasswordHasher::hash(password, credential.salt);
    return credential;
}

bool AuthenticationService::account_number_well_formed(const std::string& account_number) {
    if (account_number.size() < 4 || account_number.size() > 20) return false;
    for (char ch : account_number) {
        if (std::isalnum(static_cast<unsigned char>(ch)) == 0) return false;
    }
    return true;
}

const AccountCredential* AuthenticationService::find_credential(
    const std::string& account_number) const {
    // 账号比较不区分大小写：学生常把小写 s 打成大写 S。
    const std::string lowered = to_lower_copy(account_number);
    for (const AccountCredential& credential : credentials_) {
        if (to_lower_copy(credential.account_number) == lowered) return &credential;
    }
    return nullptr;
}

bool AuthenticationService::role_account_active(const AccountCredential& credential) const {
    switch (credential.role) {
        case Role::Student: {
            const Student* student = context_.find_student(credential.user_id);
            return student != nullptr && student->account_active;
        }
        case Role::Teacher: {
            const Teacher* teacher = context_.find_teacher(credential.user_id);
            return teacher != nullptr && teacher->account_active;
        }
        case Role::Administrator: {
            const Administrator* admin = context_.find_administrator(credential.user_id);
            return admin != nullptr && admin->account_active;
        }
    }
    return false;
}

std::string AuthenticationService::next_token(const std::string& account_number) {
    ++token_counter_;
    // 加序号，保证同一分钟内连续登录两次也不会撞令牌
    return PasswordHasher::sha256_hex(
               account_number + "|" + context_.now() + "|" +
               std::to_string(token_counter_))
        .substr(0, 32);
}

OperationResult AuthenticationService::login(const std::string& account_number,
                                             const std::string& password,
                                             UserIdentity& identity,
                                             std::string& token) {
    identity = UserIdentity{};   // 失败时不留上一次的身份
    token.clear();

    if (!account_number_well_formed(account_number) || password.empty()) {
        return {ResultCode::InvalidInput, "Account number or password is invalid"};
    }

    const std::string now = context_.now();
    AccountState& state = states_[account_number];

    // 锁定优先于一切：锁定期内即使密码正确也拒绝，否则锁定形同虚设
    if (!state.locked_until.empty() && moment_before(now, state.locked_until)) {
        return {ResultCode::AccountLocked,
                "Account is locked until " + state.locked_until};
    }
    if (!state.locked_until.empty()) {   // 锁定已过期，从头计数
        state.locked_until.clear();
        state.failures = 0;
    }

    const AccountCredential* credential = find_credential(account_number);
    const bool password_ok =
        credential != nullptr &&
        PasswordHasher::verify(password, credential->salt, credential->password_hash);

    if (!password_ok) {
        ++state.failures;
        if (state.failures >= policy_.max_failures) {
            state.locked_until = add_minutes(now, policy_.lock_minutes);
            return {ResultCode::AccountLocked,
                    "Too many failed attempts, account locked until " +
                        state.locked_until};
        }
        // 账号不存在与密码错误返回同一句话，避免用登录接口枚举学号
        return {ResultCode::AuthenticationFailed,
                "Account number or password is incorrect"};
    }

    if (!role_account_active(*credential)) {
        return {ResultCode::AccountDisabled,
                "Account is disabled, please contact the administration office"};
    }

    state.failures = 0;
    state.locked_until.clear();

    identity = UserIdentity{credential->user_id, credential->role,
                            credential->account_number, std::string()};
    token = next_token(credential->account_number);
    sessions_[token] = SessionInfo{token, identity, now,
                                   add_minutes(now, policy_.session_minutes)};
    return {ResultCode::Success, "Login succeeded"};
}

OperationResult AuthenticationService::validate_session(const std::string& token,
                                                        UserIdentity& identity) {
    identity = UserIdentity{};
    if (token.empty()) {
        return {ResultCode::SessionInvalid, "Session token is required"};
    }
    auto found = sessions_.find(token);
    if (found == sessions_.end()) {
        return {ResultCode::SessionInvalid, "Session token is unknown or logged out"};
    }
    if (!moment_before(context_.now(), found->second.expires_at)) {
        sessions_.erase(found);   // 过期即删除，避免会话表只增不减
        return {ResultCode::SessionExpired, "Session has expired, please sign in again"};
    }
    identity = found->second.user;
    return {ResultCode::Success, "Session is valid"};
}

OperationResult AuthenticationService::logout(const std::string& token) {
    auto found = sessions_.find(token);
    if (found == sessions_.end()) {
        return {ResultCode::SessionInvalid, "Session token is unknown or logged out"};
    }
    sessions_.erase(found);
    return {ResultCode::Success, "Logout succeeded"};
}

bool AuthenticationService::is_locked(const std::string& account_number) const {
    auto found = states_.find(account_number);
    if (found == states_.end() || found->second.locked_until.empty()) return false;
    return moment_before(context_.now(), found->second.locked_until);
}

int AuthenticationService::failure_count(const std::string& account_number) const {
    auto found = states_.find(account_number);
    return found == states_.end() ? 0 : found->second.failures;
}

OperationResult AuthenticationService::require_role(const UserIdentity& user, Role role) {
    if (user.role != role) {
        return {ResultCode::PermissionDenied, "This entry requires a different role"};
    }
    return {ResultCode::Success, "Role accepted"};
}

}  // namespace course_selection
