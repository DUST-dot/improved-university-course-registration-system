# Week 3 - Conflict, Review, Import, Drop, Monitoring (sections 2.10-2.14)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script, and it compiles against the shared core
that every week carries a copy of.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.10 | US-04 | Time conflict reminder | 5 | 3 | M2 | 25 |
| 2.11 | US-14 | Review Teacher Requests | 5 | 3 | M3 | 17 |
| 2.12 | US-15 | Import Course File | 5 | 3 | M3 | 27 |
| 2.13 | US-06 | drop the class | 3 | 2 | M2 | 21 |
| 2.14 | US-17 | Real-time Monitoring | 5 | 3 | M3 | 12 |

Total: **102 assertions**.

## Build and test

```powershell
.\build_and_test.ps1
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0.
The demo programs are built by the same script when present.

## Test files

```text
  us04_conflict_tests.cpp
  us14_review_tests.cpp
  us15_import_tests.cpp
  us06_drop_tests.cpp
  us17_monitor_tests.cpp
```

## Layout

```text
week3/
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
