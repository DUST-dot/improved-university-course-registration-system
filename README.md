# Improved University Course Registration System

Team 4 · C++17 implementation

The project is delivered **one folder per iteration**. Every `weekN/` folder is
self-contained: it has its own headers, sources, tests and build script, so it can be
compiled and tested on its own without touching the other weeks.

## Iterations

| Branch | Week | Sections | Stories | Tests | Diff |
|---|---|---|---|---|---|
| [`feature/week1`](../../tree/feature/week1) | Week 1 | 2.1-2.4 | US-01 Student Login, US-02 Filter Courses, US-09 Teacher Login, US-13 Admin Login | 177 | [view](../../compare/main...feature/week1) |
| [`feature/week2`](../../tree/feature/week2) | Week 2 | 2.5-2.6 | US-10 Apply to Open a New Course, US-03 Remaining Seats | 68 | [view](../../compare/main...feature/week2) |
| [`feature/week3`](../../tree/feature/week3) | Week 3 | 2.7-2.9 | US-16 Registration Window, US-11 Teacher Enrollment Count, US-12 Change Request | 94 | [view](../../compare/main...feature/week3) |

Total: **339 assertions, all passing**, no warnings under `-Wall -Wextra`.

Each branch also carries an annotated tag (`v0.1-week1`, `v0.2-week2`, `v0.3-week3`)
so any iteration can be checked out directly.

## Repository layout

```text
week1/   sections 2.1-2.4  login and course filtering
week2/   sections 2.5-2.6  course application and seat availability
week3/   sections 2.7-2.9  registration window, teacher view, change request
```

Inside every week folder:

```text
header_files/      interfaces and data structures
source_code/       implementations plus a console demonstration
automated_tests/   assertions, one executable per story
build_and_test.ps1 compiles everything and runs the tests
CMakeLists.txt     equivalent CMake configuration
README.md          task table, results and file mapping for that week
```

## Build and test

```powershell
cd week1
.\build_and_test.ps1
.\build\week1_demo.exe
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0 when everything passes.

## Test results per week

```text
week1: PASS 58 / 38 / 42 / 39      ALL BUILD AND TESTS PASSED
week2: PASS 41 / 27                ALL BUILD AND TESTS PASSED
week3: PASS 31 / 30 / 33           ALL BUILD AND TESTS PASSED
```

## Design conventions

| Convention | Reason |
|---|---|
| Every business call returns `OperationResult` | A rejected enrollment is an expected outcome, not an exception |
| Output parameters are cleared on failure | A caller that ignores the result code gets an empty value, never a stale one |
| Remaining seats are always `capacity - active`, never cached | One writable copy fewer, one whole class of inconsistency fewer |
| Time is always read from `SystemContext::now()` | Tests pin it with `set_now_for_tests`, so results do not depend on the date |
| Service entry points never take a `student_id` | The id can only come from the logged-in identity, which removes impersonation |

## Not implemented yet

- Enroll / drop / timetable (the core selection flow)
- Approval workflow US-14: the state machine is in place, change requests are not wired to it yet
- Persistence: all state lives in memory and is lost on restart
- Load testing: concurrency is correct first, throughput comes with US-19