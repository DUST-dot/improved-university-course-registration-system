# Week 4 - Feedback, Distribution, Timetable (sections 2.15-2.17)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script, and it compiles against the shared core
that every week carries a copy of.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.15 | US-05 | Course selection result feedback | 3 | 2 | M2 | 9 |
| 2.16 | US-18 | View Enrollment Distribution Chart | 5 | 3 | M3 | 10 |
| 2.17 | US-07 | Check the class schedule | 3 | 2 | M2 | 13 |

Total: **32 assertions**.

## Build and test

```powershell
.\build_and_test.ps1
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0.
The demo programs are built by the same script when present.

## Test files

```text
  us05_feedback_tests.cpp
  us18_distribution_tests.cpp
  us07_timetable_tests.cpp
```

## Layout

```text
week4/
  README.md           this file
  header_files/       domain model, shared context, permission rules, services
  source_code/        implementations
  automated_tests/    one executable per story plus the fixture header
  build_and_test.ps1  build and test entry point
```

## Notes

- The core files (`domain.hpp`, `system_context.hpp`, `time_util.hpp`,
  `permission_rules.hpp`) are identical in every week: one model, no drift.
- A service from an earlier week is copied forward only when it is a dependency
  (for example `apply_service.hpp` provides `ApplyStatus`), never to duplicate a story.
- Remaining seats are always `capacity - active`; nothing is cached, so the numbers
  on every screen agree by construction.
