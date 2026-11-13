#include "enrollment_service.hpp"

namespace course_selection {

double EnrollmentService::active_credits(int student_id) const {
    double total = 0.0;
    for (const Enrollment& record : context_.active_enrollments(student_id)) {
        TeachingClass item{};
        if (!context_.find_teaching_class(record.teaching_class_id, item)) continue;
        Course course{};
        if (!context_.find_course(item.course_id, course)) continue;
        total += course.credits;
    }
    return total;
}

OperationResult EnrollmentService::describe(int teaching_class_id, EnrollOutcome& outcome,
                                            const std::string& message) const {
    outcome.teaching_class_id = teaching_class_id;
    outcome.active_enrollments = context_.enrollment_count(teaching_class_id);
    outcome.remaining_seats = context_.remaining_seats(teaching_class_id);
    outcome.message = message;
    return {ResultCode::Success, message};
}

OperationResult EnrollmentService::enroll(int student_id, int teaching_class_id,
                                          EnrollOutcome& outcome) {
    outcome = EnrollOutcome{};

    // 1. Window. Manual switch or the configured timestamps decide.
    const RegistrationRule rule = context_.rule();
    if (!rule.registration_open) {
        return {ResultCode::RegistrationClosed, "Registration is closed"};
    }

    // 2. The class must exist.
    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::TeachingClassNotFound, "Teaching class not found"};
    }

    // 3. Already enrolled in this exact class?
    for (const Enrollment& record : context_.active_enrollments(student_id)) {
        if (record.teaching_class_id == teaching_class_id) {
            return {ResultCode::AlreadyEnrolled, "You have already enrolled in this class"};
        }
    }

    // 4. Capacity, computed on the same counter the seat view uses.
    if (context_.remaining_seats(teaching_class_id) <= 0) {
        return {ResultCode::TeachingClassFull, "This class is full, please pick another one"};
    }

    // 5. Credit cap. The subject is counted once because a duplicate was rejected above.
    Course course{};
    context_.find_course(item.course_id, course);
    if (active_credits(student_id) + course.credits > rule.maximum_credits) {
        return {ResultCode::CreditLimitExceeded, "This would exceed your credit limit"};
    }

    context_.add_enrollment(student_id, teaching_class_id);
    return describe(teaching_class_id, outcome, "Enrolled");
}

OperationResult EnrollmentService::drop(int student_id, int teaching_class_id,
                                        EnrollOutcome& outcome) {
    outcome = EnrollOutcome{};

    TeachingClass item{};
    if (!context_.find_teaching_class(teaching_class_id, item)) {
        return {ResultCode::TeachingClassNotFound, "Teaching class not found"};
    }

    // Only an Active record can be dropped; a second drop must not look successful.
    if (!context_.drop_active_enrollment(student_id, teaching_class_id)) {
        return {ResultCode::ActiveEnrollmentNotFound,
                "You are not enrolled in this teaching class"};
    }
    return describe(teaching_class_id, outcome, "Dropped");
}

}  // namespace course_selection
