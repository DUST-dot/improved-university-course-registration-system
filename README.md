# Improved University Course Registration System

Team 4 · C++17 implementation

Delivered **one folder per iteration**. Each `weekN/` holds exactly the sections that
start in that week according to the 24-task schedule, and is self-contained: its own
headers, sources, tests and build script.

## Iterations

| Branch | Sections | Stories | Assertions | Tag |
|---|---|---|---|---|
| [`feature/week1`](../../tree/feature/week1) | 2.1-2.5 | US-01 login, US-02 filter, US-09 teacher login, US-13 admin login, US-10 course application | 218 | `v1.0-week1` |
| [`feature/week2`](../../tree/feature/week2) | 2.6-2.9 | US-03 seats, US-16 window, US-11 teacher count, US-12 change request | 121 | `v1.0-week2` |
| [`feature/week3`](../../tree/feature/week3) | 2.10-2.14 | US-04 conflict, US-14 review, US-15 import, US-06 drop, US-17 monitor | 102 | `v1.0-week3` |
| [`feature/week4`](../../tree/feature/week4) | 2.15-2.17 | US-05 feedback, US-18 distribution, US-07 timetable | 32 | `v1.0-week4` |
| [`feature/week5`](../../tree/feature/week5) | 2.18 | US-08 recommendation | 12 | `v1.0-week5` |
| [`feature/week6`](../../tree/feature/week6) | 2.19-2.20 | US-19 performance, US-20 data reliability | 16 | `v1.0-week6` |
| [`feature/week8`](../../tree/feature/week8) | 2.21-2.22 | US-21 security, US-22 usability | 19 | `v1.0-week8` |
| [`feature/week9`](../../tree/feature/week9) | 2.23-2.24 | US-23 maintainability, US-24 release and buffer | 6 | `v1.0-week9` |

Total: **526 assertions, all passing**, no warnings under `-Wall -Wextra`.

All 24 sections are delivered. Weeks 7 and 10 add no new section: section 2.20
spans weeks 6-7 and 2.24 spans weeks 9-10, so their folders stay where the work starts.

## Repository layout

```text
week1/   sections 2.1-2.5
week2/   sections 2.6-2.9
week3/   sections 2.10-2.14
week4/   sections 2.15-2.17
week5/   sections 2.18
week6/   sections 2.19-2.20
week8/   sections 2.21-2.22
week9/   sections 2.23-2.24
```

Inside every week folder:

```text
header_files/      domain model, shared context, permission rules, services
source_code/       implementations
automated_tests/   one executable per story
build_and_test.ps1 compiles everything and runs the tests
README.md          task table and file mapping for that week
```

## Build and test

```powershell
cd week1
.\build_and_test.ps1
.\build\week1_demo.exe
```

Requires a C++17 compiler (developed with MinGW g++ 8.1.0 on Windows).
The script prints `ALL BUILD AND TESTS PASSED` and returns exit code 0.

## Test results

```text
week1  218 checks    week4   32 checks    week8   19 checks
week2  121 checks    week5   12 checks    week9    6 checks
week3  102 checks    week6   16 checks
```

## Design conventions

| Convention | Reason |
|---|---|
| Every business call returns `OperationResult` | A rejected enrollment is an expected outcome, not an exception |
| Output parameters are cleared on failure | A caller that ignores the result code gets an empty value, never a stale one |
| Remaining seats are always `capacity - active`, never cached | One writable copy fewer, one whole class of inconsistency fewer |
| Time is always read from `SystemContext::now()` | Tests pin it with `set_now_for_tests`, so results do not depend on the date |
| Service entry points never take a `student_id` | The id can only come from the logged-in identity, which removes impersonation |
| One shared core, copied per week | Each folder compiles on its own, and the model cannot drift between weeks |