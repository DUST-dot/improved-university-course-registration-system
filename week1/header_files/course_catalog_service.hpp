#pragma once

// ============================================================================
// 课程目录查询（US-02 筛选）
//
// 只读模块：不新增、不修改、不删除任何记录，所以不会和选课流程打架。
// 筛选条件是"可任意组合"的，在内存里逐条比对比拼 SQL 更容易把规则讲清楚。
// ============================================================================

#include <string>
#include <vector>

#include "system_context.hpp"

namespace course_selection {

// 留空或 -1 表示"这一项不参与筛选"。
// 学分区间是闭区间，两端可单独使用；teacher_id 与 teacher_keyword 同时给出时，
// 两者必须落在同一个教学班，否则会拿 A 的班凑关键词、B 的班凑筛选。
struct CourseFilter {
    std::string course_code{};
    int credits{-1};          // 精确学分
    int teacher_id{-1};
    int minimum_credits{-1};  // 学分下限（含）
    int maximum_credits{-1};  // 学分上限（含）
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

    // 关键词按课程代码、课程名称、教师姓名三处做不区分大小写的包含匹配。
    // 没有匹配时返回 Success + 空列表，由界面提示"没有匹配课程"。
    // 条件非法（学分区间下限大于上限等）返回 InvalidInput 并清空结果——
    // 空列表会被界面当成"没有符合条件的课程"，把输入错误掩盖掉。
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
