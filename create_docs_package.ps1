# Documentation Package Builder
# Creates a separate documentation package for releases

param(
    [Parameter(Mandatory=$true)]
    [string]$Version
)

Write-Host "Creating Documentation Package for v$Version..." -ForegroundColor Green

# Create docs directory
$docsDir = "release\docs-v$Version"
if (Test-Path $docsDir) {
    Remove-Item $docsDir -Recurse -Force
}
New-Item -ItemType Directory -Path $docsDir -Force | Out-Null

# Copy documentation files
Write-Host "Packaging documentation..." -ForegroundColor Yellow
Copy-Item "USER_MANUAL.md" "$docsDir\USER_MANUAL.md"
Copy-Item "README.md" "$docsDir\PROJECT_README.md"

# Copy any additional docs if they exist
if (Test-Path "OTA_UPDATE_GUIDE.md") {
    Copy-Item "OTA_UPDATE_GUIDE.md" "$docsDir\OTA_UPDATE_GUIDE.md"
}
if (Test-Path "DEPLOY.md") {
    Copy-Item "DEPLOY.md" "$docsDir\DEPLOYMENT_GUIDE.md"
}

# Create documentation index
$docIndex = @"
# ESP32 Clock Radio Documentation Package v$Version

This package contains all documentation for the ESP32 Clock Radio project.

## Documents Included:

### 📖 USER_MANUAL.md
Complete user guide covering:
- Hardware setup and first-time configuration
- Menu system navigation and all features
- Alarm system with 5 independent alarms
- OTA firmware update system
- Troubleshooting and error resolution
- Technical specifications

### 🚀 PROJECT_README.md  
Project overview and development information:
- Hardware requirements and pin configurations
- Build instructions and development setup
- Feature overview and system architecture
- Contributing guidelines

### 🔄 OTA_UPDATE_GUIDE.md (if included)
Technical guide for the OTA update system:
- Implementation details
- Testing procedures
- GitHub release requirements

### 🚀 DEPLOYMENT_GUIDE.md (if included)
Release and deployment procedures:
- Creating releases manually and automatically
- GitHub Actions workflow configuration

## Quick Links:

- **New User?** Start with USER_MANUAL.md
- **Developer?** See PROJECT_README.md  
- **Need Help?** Check troubleshooting section in USER_MANUAL.md
- **Want to Contribute?** See contributing section in PROJECT_README.md

## Version Information:

- Documentation Version: $Version
- Compatible Firmware: $Version and later
- Last Updated: $(Get-Date -Format "MMMM yyyy")

---

For the latest documentation and firmware releases, visit:
https://github.com/oosthub/Clock-Radio/releases
"@

Set-Content -Path "$docsDir\README.md" -Value $docIndex

# Create ZIP archive
Write-Host "Creating documentation ZIP..." -ForegroundColor Yellow
$zipPath = "clock-radio-docs-v$Version.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}

Compress-Archive -Path "$docsDir\*" -DestinationPath $zipPath

Write-Host "Documentation package created successfully!" -ForegroundColor Green
Write-Host "Files:"
Write-Host "  - Documentation folder: $docsDir"
Write-Host "  - ZIP archive: $zipPath"
Write-Host ""
Write-Host "Upload the documentation ZIP as an additional asset to your GitHub release." -ForegroundColor Cyan
