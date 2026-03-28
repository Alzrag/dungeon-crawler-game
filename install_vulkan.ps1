# install_vulkan.ps1
# Detects if Vulkan SDK is installed; if not, downloads from LunarG and installs.

param (
    [string]$InstallerName = "vulkansdk-windows-X64-1.4.341.1.exe",
    [string]$VulkanVersion = "1.4.341.1",
    [string]$InstallRoot   = "C:\VulkanSDK"
)

$InstallPath = "$InstallRoot\$VulkanVersion"

function Write-Status($msg) { Write-Host "[Vulkan] $msg" -ForegroundColor Cyan }
function Write-Ok($msg)     { Write-Host "[Vulkan] $msg" -ForegroundColor Green }
function Write-Err($msg)    { Write-Host "[Vulkan] ERROR: $msg" -ForegroundColor Red; exit 1 }

# ── Helper: check if validation layers are present ───────────────────────────
function Test-ValidationLayers($sdkPath) {
    # Most reliable: check for the layer JSON in the SDK Bin dir
    if (Test-Path "$sdkPath\Bin\VkLayer_khronos_validation.json") { return $true }

    # Also check the registry where the SDK registers explicit layers
    $regPaths = @(
        "HKLM:\SOFTWARE\Khronos\Vulkan\ExplicitLayers",
        "HKCU:\SOFTWARE\Khronos\Vulkan\ExplicitLayers"
    )
    foreach ($reg in $regPaths) {
        if (Test-Path $reg) {
            $names = (Get-Item -Path $reg).Property
            foreach ($name in $names) {
                if ($name -like "*khronos_validation*") { return $true }
            }
        }
    }
    return $false
}

# ── Helper: set env vars for current process and exit ────────────────────────
function Use-ExistingSDK($sdkPath) {
    $env:VULKAN_SDK = $sdkPath
    $binPath = "$sdkPath\Bin"
    if ($env:PATH -notlike "*$binPath*") { $env:PATH = "$binPath;$env:PATH" }
    $env:LIB     = "$sdkPath\Lib;$env:LIB"
    $env:INCLUDE = "$sdkPath\Include;$env:INCLUDE"
}

# ── 1. Detect existing installation ──────────────────────────────────────────
$sdkFromEnv = $env:VULKAN_SDK
if (-not $sdkFromEnv) {
    $sdkFromEnv = [System.Environment]::GetEnvironmentVariable("VULKAN_SDK", "User")
}
if (-not $sdkFromEnv) {
    $sdkFromEnv = [System.Environment]::GetEnvironmentVariable("VULKAN_SDK", "Machine")
}

if ($sdkFromEnv -and (Test-Path $sdkFromEnv)) {
    if (Test-ValidationLayers $sdkFromEnv) {
        Write-Ok "Vulkan SDK and validation layers already installed at: $sdkFromEnv"
        Use-ExistingSDK $sdkFromEnv
        exit 0
    } else {
        Write-Status "Vulkan SDK found at $sdkFromEnv but validation layers missing -- reinstalling..."
    }
} elseif (Test-Path "$InstallPath\Bin\vulkan-1.dll") {
    if (Test-ValidationLayers $InstallPath) {
        Write-Ok "Vulkan SDK found at: $InstallPath"
        Use-ExistingSDK $InstallPath
        exit 0
    } else {
        Write-Status "Vulkan SDK found at $InstallPath but validation layers missing -- reinstalling..."
    }
}

# ── 2. Download installer if not already present ──────────────────────────────
$scriptDir     = Split-Path -Parent $MyInvocation.MyCommand.Definition
$installerPath = Join-Path $scriptDir $InstallerName
$downloadUrl   = "https://sdk.lunarg.com/sdk/download/$VulkanVersion/windows/$InstallerName"

if (-not (Test-Path $installerPath)) {
    Write-Status "Installer not found locally. Downloading from LunarG..."
    Write-Status "URL: $downloadUrl"
    try {
        Import-Module BitsTransfer -ErrorAction SilentlyContinue
        if (Get-Command Start-BitsTransfer -ErrorAction SilentlyContinue) {
            Start-BitsTransfer -Source $downloadUrl -Destination $installerPath -DisplayName "Vulkan SDK $VulkanVersion"
        } else {
            $wc = New-Object System.Net.WebClient
            $wc.DownloadFile($downloadUrl, $installerPath)
        }
        Write-Ok "Download complete: $installerPath"
    } catch {
        Write-Err "Download failed: $_"
    }
} else {
    Write-Status "Installer already present: $installerPath"
}

# ── 3. Run installer silently ─────────────────────────────────────────────────
Write-Status "Installing Vulkan SDK $VulkanVersion (includes validation layers)..."
$proc = Start-Process -FilePath $installerPath `
    -ArgumentList "/S", "/D=$InstallPath" `
    -Wait -PassThru

if ($proc.ExitCode -ne 0) {
    Write-Err "Installer exited with code $($proc.ExitCode)."
}
Write-Ok "Installation completed."

# ── 4. Set environment variables ──────────────────────────────────────────────
Write-Status "Setting environment variables ..."
$binPath = "$InstallPath\Bin"
$scope   = "Machine"

try {
    [System.Environment]::SetEnvironmentVariable("VULKAN_SDK", $InstallPath, "Machine")
    $mp = [System.Environment]::GetEnvironmentVariable("PATH", "Machine")
    if ($mp -notlike "*$binPath*") {
        [System.Environment]::SetEnvironmentVariable("PATH", "$binPath;$mp", "Machine")
    }
    Write-Ok "Environment variables set machine-wide."
} catch {
    $scope = "User"
    Write-Status "No admin rights -- setting user-level environment variables."
    [System.Environment]::SetEnvironmentVariable("VULKAN_SDK", $InstallPath, "User")
    $up = [System.Environment]::GetEnvironmentVariable("PATH", "User")
    if ($up -notlike "*$binPath*") {
        [System.Environment]::SetEnvironmentVariable("PATH", "$binPath;$up", "User")
        Write-Ok "Added $binPath to user PATH."
    } else {
        Write-Ok "User PATH already contains $binPath."
    }
}

# Always update current process so this build can link immediately
Use-ExistingSDK $InstallPath

Write-Ok "VULKAN_SDK = $InstallPath [$scope]"
Write-Ok "Vulkan SDK setup complete."
