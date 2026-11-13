# Week 1 - Login and Course Application (sections 2.1-2.5)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script, and it compiles against the shared core
that every week carries a copy of.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.1 | US-01 | Student Login | 5 | 3 | M1 | 58 |
| 2.2 | US-02 | Filter Courses | 5 | 3 | M1 | 38 |
| 2.3 | US-09 | Teacher Login | 2 | 1 | M1 | 42 |
| 2.4 | US-13 | Admin Back-office Login | 2 | 1 | M1 | 39 |
| 2.5 | US-10 | Apply to Open a New Course | 5 | 3 | M3 | 41 |

Total: **218 assertions**.

## Build and test

```powershell
.\build_and_test.ps1
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0.
The demo programs are built by the same script when present.

## Test files

```text
  us01_student_login_tests.cpp
  us02_course_filter_tests.cpp
  us09_teacher_login_tests.cpp
  us13_admin_login_tests.cpp
  us10_apply_service_tests.cpp
```

## Layout

```text
week1/
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
