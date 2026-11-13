# Week 6 - Performance and Data Reliability (sections 2.19-2.20)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script, and it compiles against the shared core
that every week carries a copy of.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.19 | US-19 | Performance Reliability | 5 | 3 | M4 | 9 |
| 2.20 | US-20 | Data Reliability | 8 | 6 | M4 | 7 |

Total: **16 assertions**.

## Build and test

```powershell
.\build_and_test.ps1
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0.
The demo programs are built by the same script when present.

## Test files

```text
  us19_performance_tests.cpp
  us20_integrity_tests.cpp
```

## Layout

```text
week6/
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
