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

# ── Helper: check if validation layers are present ───────────────────────────
validation_layers_present() {
    # Check known JSON manifest locations (this is how Vulkan actually finds layers)
    local search_dirs=(
        /usr/share/vulkan/explicit_layer.d
        /usr/local/share/vulkan/explicit_layer.d
        /etc/vulkan/explicit_layer.d
    )
    for dir in "${search_dirs[@]}"; do
        if [[ -f "$dir/VkLayer_khronos_validation.json" ]]; then
            return 0
        fi
    done

    # Check if the actual .so exists anywhere on the system
    if find /usr /usr/local 2>/dev/null -name "libVkLayer_khronos_validation.so*" -quit | grep -q .; then
        return 0
    fi

    # Check via package manager as a fallback
    if command -v dpkg &>/dev/null; then
        if dpkg -l vulkan-validationlayers 2>/dev/null | grep -q "^ii"; then
            return 0
        fi
    elif command -v rpm &>/dev/null; then
        if rpm -q vulkan-validation-layers &>/dev/null; then
            return 0
        fi
    elif command -v pacman &>/dev/null; then
        if pacman -Q vulkan-validation-layers &>/dev/null; then
            return 0
        fi
    fi

    return 1
}

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
    if validation_layers_present; then
        ok "Vulkan and validation layers already installed. Nothing to do."
        exit 0
    else
        info "Vulkan found but validation layers missing — installing layers only..."
    fi
else
    info "Vulkan not detected — installing via system package manager ..."
fi

# ── 2. Detect package manager and install ────────────────────────────────────
# Packages installed:
#   - Vulkan runtime loader (libvulkan)
#   - Vulkan headers         (for compilation)
#   - Vulkan tools           (vulkaninfo, vkcube, etc.)
#   - Validation layers      (runtime + dev)
#   - GLSLC / glslang        (GLSL → SPIR-V compiler)

if command -v apt-get &>/dev/null; then
    # ── Debian / Ubuntu ───────────────────────────────────────────────────────
    info "Detected APT (Debian/Ubuntu)"
    sudo apt-get update -qq
    sudo apt-get install -y \
        libvulkan1 \
        libvulkan-dev \
        vulkan-tools \
        vulkan-validationlayers \
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
