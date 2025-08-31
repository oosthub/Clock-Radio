# Release Deployment Guide

## The GitHub Actions 403 Error Fix

The 403 error occurs because GitHub Actions doesn't have permission to create releases. Here's how to fix it:

### Step 1: Fix Repository Settings

1. **Go to your GitHub repository settings**:
   - Navigate to `https://github.com/oosthub/Clock-Radio/settings`
   - Click on "Actions" in the left sidebar
   - Click on "General"

2. **Check Workflow Permissions**:
   - Scroll down to "Workflow permissions"
   - Select "Read and write permissions"
   - Check "Allow GitHub Actions to create and approve pull requests"
   - Click "Save"

### Step 2: Update and Push the Fixed Workflow

The workflow has been updated with proper permissions. Commit and push the changes:

```bash
# Commit the workflow fixes
git add .
git commit -m "Fix GitHub Actions permissions for releases"
git push origin main
```

### Step 3: Create a New Release

Delete the failed tag and create a new one:

```bash
# Delete the failed tag locally and remotely
git tag -d v1.0.0
git push origin --delete v1.0.0

# Create a new tag
git tag v1.0.1
git push origin v1.0.1

# GitHub Actions will automatically build and create the release
```

## Alternative: Manual Release (Immediate Solution)

If you want to create a release right now while fixing the automation:

### 1. Build the Release Package

```powershell
# Run the build script
.\create_release.ps1 -Version "1.0.0"
```

### 2. Create GitHub Release Manually

1. Go to: `https://github.com/oosthub/Clock-Radio/releases`
2. Click "Create a new release"
3. **Tag**: `v1.0.0`
4. **Title**: `ESP32 Clock Radio v1.0.0`
5. **Description**:
   ```markdown
   ## ESP32 Clock Radio v1.0.0
   
   ### Features
   - Internet radio streaming with web configuration
   - 5-alarm system with fade-in functionality
   - Sleep timer and weather integration
   - Ready-to-flash firmware for ESP32-S3
   
   ### Installation
   1. Download the ZIP file below
   2. Extract files
   3. Install esptool: `pip install esptool`
   4. Connect ESP32-S3 and put in download mode
   5. Run the included flash script
   ```
6. Upload `clock-radio-v1.0.0.zip`
7. Click "Publish release"

## Future Automated Releases

Once the permissions are fixed, create releases with:

```bash
git tag v1.0.2
git push origin v1.0.2
```