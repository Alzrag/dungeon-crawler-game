#!/usr/bin/env bash
# install_vulkan.sh
# Detects if Vulkan SDK/tools are installed; if not, installs via the
# system package manager (apt, dnf, or pacman).

set -euo pipefail

# ── Colour helpers ────────────────────────────────────────────────────────────
cyan="\033[0;36m"; green="\033[0;32m"; red="\033[0;31m"; reset="\033[0m"
info()  { echo -e "${cyan}[Vulkan] $*${reset}"; }
ok()    { echo -e "${green}[Vulkan] $*${reset}"; }
err()   { echo -e "${red}[Vulkan] ERROR: $*${reset}" >&2; exit 1; }

# ── 1. Detect existing installation ──────────────────────────────────────────
vulkan_detected=0

# a) vulkaninfo tool present
if command -v vulkaninfo &>/dev/null; then
    vulkan_detected=1
fi

# b) libvulkan shared library on the linker path
if [[ $vulkan_detected -eq 0 ]] && ldconfig -p 2>/dev/null | grep -q libvulkan; then
    vulkan_detected=1
fi

# c) VULKAN_SDK env var points to a real directory
if [[ $vulkan_detected -eq 0 && -n "${VULKAN_SDK:-}" && -d "${VULKAN_SDK}" ]]; then
    vulkan_detected=1
fi

if [[ $vulkan_detected -eq 1 ]]; then
    ok "Vulkan is already installed. Nothing to do."
    exit 0
fi

info "Vulkan not detected — installing via system package manager ..."

# ── 2. Detect package manager and install ────────────────────────────────────
# Packages installed:
#   - Vulkan runtime loader (libvulkan)
#   - Vulkan headers         (for compilation)
#   - Vulkan tools           (vulkaninfo, vkcube, etc.)
#   - Validation layers      (optional but useful for development)
#   - GLSLC / glslang        (GLSL → SPIR-V compiler)

if command -v apt-get &>/dev/null; then
    # ── Debian / Ubuntu ───────────────────────────────────────────────────────
    info "Detected APT (Debian/Ubuntu)"
    sudo apt-get update -qq
    sudo apt-get install -y \
        libvulkan1 \
        libvulkan-dev \
        vulkan-tools \
        vulkan-validationlayers-dev \
        glslang-tools \
        spirv-tools

elif command -v dnf &>/dev/null; then
    # ── Fedora / RHEL / Rocky / Alma ─────────────────────────────────────────
    info "Detected DNF (Fedora/RHEL family)"
    sudo dnf install -y \
        vulkan-loader \
        vulkan-headers \
        vulkan-tools \
        vulkan-validation-layers \
        glslang \
        spirv-tools

elif command -v pacman &>/dev/null; then
    # ── Arch / Manjaro ────────────────────────────────────────────────────────
    info "Detected Pacman (Arch/Manjaro)"
    sudo pacman -Sy --noconfirm \
        vulkan-icd-loader \
        vulkan-headers \
        vulkan-tools \
        vulkan-validation-layers \
        glslang \
        spirv-tools

else
    err "No supported package manager found (apt, dnf, pacman). Install Vulkan manually."
fi

ok "Vulkan packages installed."

# ── 3. Set VULKAN_SDK environment variable ────────────────────────────────────
# On Linux the Vulkan SDK is not a single versioned directory like Windows;
# headers/libs live under standard system paths. We expose a convenience var
# pointing to the pkg-config prefix so build systems can locate it.

VULKAN_SDK_PREFIX="/usr"

# Prefer the LunarG SDK if it was installed to /opt
for candidate in /opt/VulkanSDK /opt/vulkan-sdk; do
    if [[ -d "$candidate" ]]; then
        # Pick the newest version sub-directory
        latest=$(ls -1 "$candidate" 2>/dev/null | sort -V | tail -1)
        [[ -n "$latest" ]] && VULKAN_SDK_PREFIX="$candidate/$latest" && break
    fi
done

PROFILE_LINE="export VULKAN_SDK=\"$VULKAN_SDK_PREFIX\""
PROFILE_LINE_PATH='export PATH="$VULKAN_SDK/bin:$PATH"'

# Write to shell profile files that exist
for profile in "$HOME/.bashrc" "$HOME/.zshrc" "$HOME/.profile"; do
    if [[ -f "$profile" ]]; then
        if ! grep -q "VULKAN_SDK" "$profile"; then
            {
                echo ""
                echo "# Vulkan SDK — added by install_vulkan.sh"
                echo "$PROFILE_LINE"
                echo "$PROFILE_LINE_PATH"
            } >> "$profile"
            ok "Added VULKAN_SDK to $profile"
        else
            info "VULKAN_SDK already set in $profile — skipping."
        fi
    fi
done

# Export for the current process (so the calling Makefile sees it)
export VULKAN_SDK="$VULKAN_SDK_PREFIX"
export PATH="$VULKAN_SDK/bin:$PATH"

ok "VULKAN_SDK = $VULKAN_SDK"
ok "Vulkan SDK setup complete. Re-source your shell profile or open a new terminal."