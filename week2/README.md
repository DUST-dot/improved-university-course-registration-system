# Week 2 - Seats, Registration Window, Teacher Count, Change Request (sections 2.6-2.9)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script, and it compiles against the shared core
that every week carries a copy of.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.6 | US-03 | View Real-time Remaining Seats | 5 | 3 | M1 | 27 |
| 2.7 | US-16 | Set Course Start / End Time | 3 | 2 | M3 | 31 |
| 2.8 | US-11 | View Real-time Enrollment Count | 3 | 2 | M1 | 30 |
| 2.9 | US-12 | Submit Change / Cancellation Request | 3 | 2 | M3 | 33 |

Total: **121 assertions**.

## Build and test

```powershell
.\build_and_test.ps1
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0.
The demo programs are built by the same script when present.

## Test files

```text
  us03_seats_tests.cpp
  us16_window_tests.cpp
  us11_teacher_tests.cpp
  us12_change_request_tests.cpp
```

## Layout

```text
week2/
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
