# PowerShell script to make all public GitHub repos private for a specific user
# Requires: GitHub CLI (gh) to be installed and authenticated

$Username = "privuser2738"

Write-Host "Fetching public repositories for $Username..." -ForegroundColor Cyan

# Get all public repos for the user
$reposJson = gh repo list $Username --visibility public --json name --limit 1000
$repos = $reposJson | ConvertFrom-Json

if (-not $repos -or $repos.Count -eq 0) {
    Write-Host "No public repositories found for $Username" -ForegroundColor Yellow
    exit 0
}

Write-Host "Found the following public repositories:" -ForegroundColor Green
foreach ($repo in $repos) {
    Write-Host "  - $($repo.name)"
}

Write-Host ""
$confirm = Read-Host "Do you want to make all these repositories private? (y/N)"

if ($confirm -ne "y" -and $confirm -ne "Y") {
    Write-Host "Operation cancelled." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "Making repositories private..." -ForegroundColor Cyan

$successCount = 0
$failCount = 0

foreach ($repo in $repos) {
    $repoName = $repo.name
    $fullName = "$Username/$repoName"

    Write-Host "Setting $fullName to private..." -NoNewline

    try {
        gh repo edit $fullName --visibility private --accept-visibility-change-consequences 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Host " Done" -ForegroundColor Green
            $successCount++
        } else {
            Write-Host " Failed" -ForegroundColor Red
            $failCount++
        }
    } catch {
        Write-Host " Failed: $_" -ForegroundColor Red
        $failCount++
    }
}

Write-Host ""
Write-Host "Completed!" -ForegroundColor Cyan
Write-Host "  Successful: $successCount" -ForegroundColor Green
Write-Host "  Failed: $failCount" -ForegroundColor $(if ($failCount -gt 0) { "Red" } else { "Green" })
