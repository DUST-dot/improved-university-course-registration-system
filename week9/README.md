# Week 9 - Maintainability and Release (sections 2.23-2.24)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.23 | US-23 | Maintainability | 5 | 3 | M5 | 6 |
| 2.24 | US-24 | Release and Buffer | 3 | 2 | M5 | — |

Total: **6 assertions** covering both stories through one release check program.

## Build and test

```powershell
.\build_and_test.ps1
```

## What the tests pin down

- **Maintainability is a check, not a claim**: a newcomer should be able to build
  and verify from the README alone, so the check program runs the same
  invariants the project depends on instead of describing them in prose.
- **Release is one command**: `ReleaseCheck::run()` verifies catalogue
  consistency, data integrity, and that seats plus active count equals capacity
  everywhere. It returns a non-zero exit code on any problem, so CI can gate on
  it, and it explains every failure instead of stopping at the first.
- A deliberately broken catalogue is fed in to prove the check fails rather than
  passing everything.

## Layout

```text
week9/
  header_files/       domain model, shared context, integrity and release check
  source_code/        implementations
  automated_tests/    the release check test plus the fixture header
  build_and_test.ps1  build and test entry point
```