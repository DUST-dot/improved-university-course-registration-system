# Build and run every test of this iteration.
# Sections: 2.15,2.16,2.17
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root
$gxx = if ($env:CXX) { $env:CXX } else { "g++" }
New-Item -ItemType Directory -Force -Path build | Out-Null
$common = @("-std=c++17", "-Wall", "-Wextra", "-Iheader_files", "-Iautomated_tests")
$core = @("source_code/system_context.cpp", "source_code/time_util.cpp", "source_code/feedback_service.cpp", "source_code/analytics_service.cpp", "source_code/timetable_service.cpp")
$targets = @(@{Name="us05_feedback_tests";Source="automated_tests/us05_feedback_tests.cpp"},
    @{Name="us18_distribution_tests";Source="automated_tests/us18_distribution_tests.cpp"},
    @{Name="us07_timetable_tests";Source="automated_tests/us07_timetable_tests.cpp"})
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
