// 2.1 US-01 学生登录
//
// 覆盖：SHA-256 标准向量、密码哈希与格式、学生登录成功、失败不区分账号与密码、
// 连续 5 次失败锁定与 14/15 分钟边界、停用账号、会话有效期与注销、输入校验。

#include "test_support.hpp"

using namespace course_selection;
using test_support::check;
using test_support::check_code;

int main() {
    // ---- SHA-256 标准向量：先证明哈希实现对，再谈密码校验 ----
    check(PasswordHasher::sha256_hex("") ==
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "SHA-256 of empty string");
    check(PasswordHasher::sha256_hex("abc") ==
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
          "SHA-256 of abc");
    check(PasswordHasher::sha256_hex(
              "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
          "SHA-256 of the NIST 56-byte sample");
    check(PasswordHasher::sha256_hex(std::string(64, 'a')).size() == 64,
          "SHA-256 handles a full block");

    // ---- 密码哈希与格式 ----
    const std::string salt = PasswordHasher::make_salt("S2023001");
    check(salt.size() == 16, "salt is 16 hex characters");
    check(salt != PasswordHasher::make_salt("S2023002"), "different accounts differ in salt");
    const std::string digest = PasswordHasher::hash("Abcd1234", salt);
    check(digest == PasswordHasher::hash("Abcd1234", salt), "hash is deterministic");
    check(digest.size() == 64, "hash is 64 hex characters");
    check(PasswordHasher::verify("Abcd1234", salt, digest), "correct password verifies");
    check(!PasswordHasher::verify("Abcd12345", salt, digest), "wrong password rejected");
    check(PasswordHasher::hash("Abcd1234", PasswordHasher::make_salt("S2023002")) != digest,
          "same password with another salt hashes differently");

    check(PasswordHasher::acceptable_password("Abcd1234"), "Abcd1234 acceptable");
    check(!PasswordHasher::acceptable_password("abcdefgh"), "letters only rejected");
    check(!PasswordHasher::acceptable_password("12345678"), "digits only rejected");
    check(!PasswordHasher::acceptable_password("Ab1"), "too short rejected");
    check(PasswordHasher::acceptable_password("Abcdefgh123456789012"), "20 chars accepted");
    check(!PasswordHasher::acceptable_password("Abcdefgh1234567890123"), "21 chars rejected");
    check(!PasswordHasher::acceptable_password("Abcd 1234"), "space rejected");

    // ---- 账号格式 ----
    check(AuthenticationService::account_number_well_formed("S2023001"), "S2023001 well formed");
    check(!AuthenticationService::account_number_well_formed("S23"), "too short rejected");
    check(!AuthenticationService::account_number_well_formed("S2023 001"), "space rejected");
    check(!AuthenticationService::account_number_well_formed(""), "empty rejected");

    // ---- 登录成功：身份可直接交给业务模块 ----
    SystemContext context = test_support::make_context();
    context.set_now_for_tests("2026-10-05T09:00");
    AuthenticationService auth(context, test_support::credentials());

    UserIdentity identity;
    std::string token;
    check_code(auth.login("S2023001", "Abcd1234", identity, token).code, ResultCode::Success,
               "student login succeeds");
    check(identity.user_id == 1001, "identity keeps student_id");
    check(identity.account_number == "S2023001", "identity keeps number");
    check(identity.role == Role::Student, "identity has Student role");
    check(!token.empty(), "login issues a token");
    check(!auth.is_locked("S2023001"), "successful login leaves the account unlocked");

    // 两次登录令牌必须不同，否则同一分钟内所有人共用一个令牌
    std::string first_token;
    std::string second_token;
    auth.login("S2023001", "Abcd1234", identity, first_token);
    auth.login("S2023001", "Abcd1234", identity, second_token);
    check(!first_token.empty() && !second_token.empty() && first_token != second_token,
          "two logins produce different tokens");

    // ---- 失败：账号不存在与密码错误返回同一句话 ----
    OperationResult wrong = auth.login("S2023001", "Wrong1234", identity, token);
    check_code(wrong.code, ResultCode::AuthenticationFailed, "wrong password fails");
    check(identity.user_id == 0 && token.empty(), "failed login clears identity and token");
    check(auth.failure_count("S2023001") == 1, "failure counter increments");

    OperationResult unknown = auth.login("S2099999", "Abcd1234", identity, token);
    check_code(unknown.code, ResultCode::AuthenticationFailed, "unknown account fails");
    check(unknown.message == wrong.message,
          "unknown account and wrong password report the same message");

    // ---- 锁定：5 次失败锁 15 分钟 ----
    for (int i = 0; i < 4; ++i) {
        auth.login("S2023002", "Nope1234", identity, token);
    }
    check(auth.failure_count("S2023002") == 4, "four failures recorded");
    check(!auth.is_locked("S2023002"), "four failures is not locked yet");
    check_code(auth.login("S2023002", "Nope1234", identity, token).code, ResultCode::AccountLocked,
               "fifth failure locks the account");
    check(auth.is_locked("S2023002"), "account is locked after five failures");
    check_code(auth.login("S2023002", "Bob12345", identity, token).code, ResultCode::AccountLocked,
               "correct password rejected while locked");

    context.set_now_for_tests("2026-10-05T09:14");
    check_code(auth.login("S2023002", "Bob12345", identity, token).code, ResultCode::AccountLocked,
               "still locked at minute 14");
    context.set_now_for_tests("2026-10-05T09:15");
    check_code(auth.login("S2023002", "Bob12345", identity, token).code, ResultCode::Success,
               "unlocked at minute 15");
    check(auth.failure_count("S2023002") == 0, "failure counter resets after success");
    check(!auth.is_locked("S2023002"), "account is no longer locked");

    // ---- 停用账号 ----
    context.set_now_for_tests("2026-10-05T10:00");
    check_code(auth.login("S2023003", "Carol123", identity, token).code, ResultCode::AccountDisabled,
               "disabled account is rejected");
    check(identity.user_id == 0, "disabled account yields no identity");
    check_code(auth.login("S2023003", "Wrong123", identity, token).code,
               ResultCode::AuthenticationFailed,
               "disabled account with wrong password looks like a normal failure");

    // ---- 会话 ----
    context.set_now_for_tests("2026-10-05T11:00");
    check_code(auth.login("S2023001", "Abcd1234", identity, token).code, ResultCode::Success,
               "login before session checks");
    UserIdentity from_session;
    check_code(auth.validate_session(token, from_session).code, ResultCode::Success,
               "fresh token is valid");
    check(from_session.user_id == 1001, "session carries the identity");
    context.set_now_for_tests("2026-10-05T12:59");
    check_code(auth.validate_session(token, from_session).code, ResultCode::Success,
               "valid just before two hours");
    context.set_now_for_tests("2026-10-05T13:00");
    check_code(auth.validate_session(token, from_session).code, ResultCode::SessionExpired,
               "expired at two hours");

    std::string logout_token;
    auth.login("S2023001", "Abcd1234", identity, logout_token);
    check_code(auth.logout(logout_token).code, ResultCode::Success, "logout succeeds");
    check_code(auth.validate_session(logout_token, from_session).code, ResultCode::SessionInvalid,
               "token invalid after logout");
    check_code(auth.logout(logout_token).code, ResultCode::SessionInvalid,
               "logging out twice is reported");
    check_code(auth.validate_session("not-a-real-token", from_session).code,
               ResultCode::SessionInvalid, "unknown token is invalid");

    // ---- 输入校验 ----
    check_code(auth.login("", "Abcd1234", identity, token).code, ResultCode::InvalidInput,
               "empty account is rejected");
    check_code(auth.login("S2023001", "", identity, token).code, ResultCode::InvalidInput,
               "empty password is rejected");
    check_code(auth.login("S2023001S2023001S2023001", "Abcd1234", identity, token).code,
               ResultCode::InvalidInput, "over-long account is rejected");

    return test_support::finish();
}
