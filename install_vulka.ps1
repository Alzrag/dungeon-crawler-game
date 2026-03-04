# install_vulkan.ps1
# Detects if Vulkan SDK is installed; if not, installs it and sets environment variables.
# Assumes the Vulkan SDK installer .exe is in the same directory as this script.

param (
    [string]$InstallerName = "vulkansdk-windows-X64-1.4.341.1.exe",
    [string]$VulkanVersion = "1.4.341.1",
    [string]$InstallRoot  = "C:\VulkanSDK"
)

$InstallPath = "$InstallRoot\$VulkanVersion"

# ── Helper ───────────────────────────────────────────────────────────────────
function Write-Status($msg) { Write-Host "[Vulkan] $msg" -ForegroundColor Cyan }
function Write-Ok($msg)     { Write-Host "[Vulkan] $msg" -ForegroundColor Green }
function Write-Err($msg)    { Write-Host "[Vulkan] ERROR: $msg" -ForegroundColor Red; exit 1 }

# ── 1. Detect existing installation ─────────────────────────────────────────
$alreadyInstalled = $false

# Check environment variable
if ($env:VULKAN_SDK -and (Test-Path $env:VULKAN_SDK)) {
    $alreadyInstalled = $true
    Write-Ok "Vulkan SDK already installed at: $($env:VULKAN_SDK)"
}

# Fallback: check default install path for this version
if (-not $alreadyInstalled -and (Test-Path "$InstallPath\Bin\vulkan-1.dll")) {
    $alreadyInstalled = $true
    Write-Ok "Vulkan SDK found at default path: $InstallPath"
}

# Fallback: check registry
if (-not $alreadyInstalled) {
    $regKey = "HKLM:\SOFTWARE\Khronos\Vulkan\ExplicitLayers"
    if (Test-Path $regKey) {
        $layers = Get-ItemProperty $regKey -ErrorAction SilentlyContinue
        if ($layers) {
            $alreadyInstalled = $true
            Write-Ok "Vulkan runtime detected via registry."
        }
    }
}

if ($alreadyInstalled) {
    Write-Ok "Vulkan SDK is already present. Nothing to do."
    exit 0
}

# ── 2. Run installer ─────────────────────────────────────────────────────────
$scriptDir    = Split-Path -Parent $MyInvocation.MyCommand.Definition
$installerPath = Join-Path $scriptDir $InstallerName

if (-not (Test-Path $installerPath)) {
    Write-Err "Installer not found: $installerPath`nPlace $InstallerName in the same folder as this script."
}

Write-Status "Installing Vulkan SDK $VulkanVersion ..."
Write-Status "Installer: $installerPath"

# Silent install (NSIS-based installer flags)
$proc = Start-Process -FilePath $installerPath `
    -ArgumentList "/S", "/D=$InstallPath" `
    -Wait -PassThru

if ($proc.ExitCode -ne 0) {
    Write-Err "Installer exited with code $($proc.ExitCode). Installation failed."
}

Write-Ok "Installation completed."

# ── 3. Set environment variables (machine-wide, persists across sessions) ────
Write-Status "Setting environment variables ..."

[System.Environment]::SetEnvironmentVariable("VULKAN_SDK", $InstallPath, "Machine")

$currentPath = [System.Environment]::GetEnvironmentVariable("PATH", "Machine")
$binPath = "$InstallPath\Bin"

if ($currentPath -notlike "*$binPath*") {
    [System.Environment]::SetEnvironmentVariable("PATH", "$binPath;$currentPath", "Machine")
    Write-Ok "Added $binPath to system PATH."
} else {
    Write-Ok "PATH already contains $binPath."
}

# Also update the current process environment so the rest of the build can use it immediately
$env:VULKAN_SDK = $InstallPath
$env:PATH       = "$binPath;$env:PATH"

Write-Ok "VULKAN_SDK = $InstallPath"
Write-Ok "Vulkan SDK setup complete."