# Week 8 - Security and Usability (sections 2.21-2.22)

This folder is the complete delivery for its week. It is self-contained: its own
headers, sources, tests and build script.

## Tasks

| Section | Story | Title | Points | Days | Milestone | Checks |
|---|---|---|---|---|---|---|
| 2.21 | US-21 | Security | 5 | 3 | M4 | 10 |
| 2.22 | US-22 | Usability | 3 | 2 | M4 | 9 |

Total: **19 assertions**.

## Build and test

```powershell
.\build_and_test.ps1
```

## What the tests pin down

- **Security is structural first**: the enrollment entry point never takes a
  `student_id`, so a caller cannot act for somebody else even with a crafted
  request. What remains is checked explicitly: horizontal (a student may only
  touch their own records) and vertical (a teacher may only touch their own
  classes), plus an append-only audit trail.
- **Usability is one sentence per failure**: every code has a hint a student can
  act on, and forms are validated before they reach the service layer.

## Layout

```text
week8/
  header_files/       domain model, shared context, security and usability services
  source_code/        implementations
  automated_tests/    one executable per story plus the fixture header
  build_and_test.ps1  build and test entry point
```