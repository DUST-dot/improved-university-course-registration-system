# Build and run every test of this iteration.
# Sections: 2.10,2.11,2.12,2.13,2.14
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root
$gxx = if ($env:CXX) { $env:CXX } else { "g++" }
New-Item -ItemType Directory -Force -Path build | Out-Null
$common = @("-std=c++17", "-Wall", "-Wextra", "-Iheader_files", "-Iautomated_tests")
$core = @("source_code/system_context.cpp", "source_code/time_util.cpp", "source_code/conflict_service.cpp", "source_code/review_service.cpp", "source_code/import_service.cpp", "source_code/enrollment_service.cpp", "source_code/monitor_service.cpp", "source_code/apply_service.cpp")
$targets = @(@{Name="us04_conflict_tests";Source="automated_tests/us04_conflict_tests.cpp"},
    @{Name="us14_review_tests";Source="automated_tests/us14_review_tests.cpp"},
    @{Name="us15_import_tests";Source="automated_tests/us15_import_tests.cpp"},
    @{Name="us06_drop_tests";Source="automated_tests/us06_drop_tests.cpp"},
    @{Name="us17_monitor_tests";Source="automated_tests/us17_monitor_tests.cpp"})
$failures = 0
foreach ($t in $targets) {
    Write-Host ("`n=== build " + $t.Name + " ===") -ForegroundColor Cyan
    & $gxx @common @($core + @($t.Source)) -o ("build/" + $t.Name + ".exe")
    if ($LASTEXITCODE -ne 0) { Write-Host ("BUILD FAILED: " + $t.Name) -ForegroundColor Red; $failures++ }
}
foreach ($t in $targets) {
    $exe = "build/" + $t.Name + ".exe"
    if (-not (Test-Path $exe)) { continue }
    Write-Host ("`n=== run " + $t.Name + " ===") -ForegroundColor Cyan
    & $exe
    if ($LASTEXITCODE -ne 0) { Write-Host ("TEST FAILED: " + $t.Name) -ForegroundColor Red; $failures++ }
}
Write-Host ""
if ($failures -eq 0) { Write-Host "ALL BUILD AND TESTS PASSED" -ForegroundColor Green; exit 0 }
Write-Host ("FAILURES: " + $failures) -ForegroundColor Red
exit 1
