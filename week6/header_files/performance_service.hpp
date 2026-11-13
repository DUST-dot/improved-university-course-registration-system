#pragma once
// 2.19 US-19 Performance reliability.
// The measurable claim: counting seats by scanning every enrollment is O(n), and
// an indexed counter makes it O(1). The service exposes both paths so the test
// can compare them and the demo can print the ratio.
#include <cstddef>
#include <unordered_map>

#include "system_context.hpp"

namespace course_selection {

class PerformanceService {
public:
    explicit PerformanceService(const SystemContext& context) : context_(context) {}

    // O(n) scan, the naive implementation.
    int count_by_scan(int teaching_class_id) const;

    // O(1) lookup after one pass over all records.
    void rebuild_index();
    int count_by_index(int teaching_class_id) const;

    std::size_t records_scanned_by_scan() const { return scanned_; }
    void reset_counters() { scanned_ = 0; calls_ = 0; }
    std::size_t index_calls() const { return calls_; }

private:
    const SystemContext& context_;
    std::unordered_map<int, int> index_;
    mutable std::size_t scanned_{0};
    mutable std::size_t calls_{0};
};

}  // namespace course_selection
