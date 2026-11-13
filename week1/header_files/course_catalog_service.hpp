#pragma once

// ============================================================================
// US-02 section
//
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

struct CourseFilter {
    std::string course_code{};
    int credits{-1};
    int teacher_id{-1};
    int minimum_credits{-1};
    int maximum_credits{-1};
    std::string teacher_keyword{};
};

struct TeachingClassView {
    int teaching_class_id;
    int teacher_id;
    std::string teacher_name;
    int capacity;
    int active_enrollments;
    int remaining_seats;
    std::vector<ClassMeeting> meetings;
};

struct CourseView {
    Course course;
    std::vector<TeachingClassView> teaching_classes;
};

class CourseCatalogService {
public:
    explicit CourseCatalogService(const SystemContext& context);

    OperationResult search(const std::string& keyword,
                           const CourseFilter& filter,
                           std::vector<CourseView>& result) const;

    OperationResult detail(int course_id, CourseView& result) const;

private:
    CourseView build_view(const Course& course, const CourseFilter& filter) const;
    bool course_matches(const Course& course,
                        const std::string& lowered_keyword,
                        const CourseFilter& filter) const;
    bool class_passes_teacher_filter(const TeachingClass& item,
                                     const CourseFilter& filter) const;
    bool filter_is_valid(const CourseFilter& filter) const;

    const SystemContext& context_;
};

}  // namespace course_selection
