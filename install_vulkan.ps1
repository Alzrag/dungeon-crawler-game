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

# 1. Detect existing installation
# Check process env var (set by a previous run in this session)
$sdkFromEnv = $env:VULKAN_SDK
# Also read from persistent user-level store (set by a previous make run)
if (-not $sdkFromEnv) {
    $sdkFromEnv = [System.Environment]::GetEnvironmentVariable("VULKAN_SDK", "User")
}
if (-not $sdkFromEnv) {
    $sdkFromEnv = [System.Environment]::GetEnvironmentVariable("VULKAN_SDK", "Machine")
}

if ($sdkFromEnv -and (Test-Path $sdkFromEnv)) {
    Write-Ok "Vulkan SDK already installed at: $sdkFromEnv"
    # Make sure current process env is populated so the linker can use it
    $env:VULKAN_SDK = $sdkFromEnv
    $binPath = "$sdkFromEnv\Bin"
    if ($env:PATH -notlike "*$binPath*") { $env:PATH = "$binPath;$env:PATH" }
    $env:LIB     = "$sdkFromEnv\Lib;$env:LIB"
    $env:INCLUDE = "$sdkFromEnv\Include;$env:INCLUDE"
    exit 0
}

if (Test-Path "$InstallPath\Bin\vulkan-1.dll") {
    Write-Ok "Vulkan SDK found at: $InstallPath"
    $env:VULKAN_SDK = $InstallPath
    $env:PATH       = "$InstallPath\Bin;$env:PATH"
    $env:LIB        = "$InstallPath\Lib;$env:LIB"
    $env:INCLUDE    = "$InstallPath\Include;$env:INCLUDE"
    exit 0
}

# 2. Download installer if not already present
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

# 3. Run installer silently
Write-Status "Installing Vulkan SDK $VulkanVersion ..."
$proc = Start-Process -FilePath $installerPath `
    -ArgumentList "/S", "/D=$InstallPath" `
    -Wait -PassThru

if ($proc.ExitCode -ne 0) {
    Write-Err "Installer exited with code $($proc.ExitCode)."
}
Write-Ok "Installation completed."

# 4. Set environment variables
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
    Write-Status "No admin rights - setting user-level environment variables."
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
$env:VULKAN_SDK = $InstallPath
$env:PATH       = "$binPath;$env:PATH"
$env:LIB        = "$InstallPath\Lib;$env:LIB"
$env:INCLUDE    = "$InstallPath\Include;$env:INCLUDE"

Write-Ok "VULKAN_SDK = $InstallPath [$scope]"
Write-Ok "Vulkan SDK setup complete."