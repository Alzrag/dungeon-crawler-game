#project name
PROJECT_NAME=main
#source file location
SOURCE= main.cpp Engine.cpp helpers.cpp GameObject.cpp staticobject.cpp fixed.cpp
#any additional header files you need
H_FILES = vertex_data.hpp helpers.h Engine.h GameObject.h staticobject.h fixed.h
#any additional resource files you need
RSR_FILES = 
#your name
NAME=Clark_Wallace

CXX = g++
CC  = gcc
CXX_FLAGS = -Wall -Wextra -Wconversion -Wdouble-promotion -Wunreachable-code -Wshadow -Wpedantic \
            -Iexternal/glfw-3.4/include -Iexternal/Vulkan-Headers/include -Iexternal
CPPVERSION = -std=c++17
OBJECTS    = $(SOURCE:.cpp=.o)
GLFW_SRC   = external/glfw-3.4/src
GLFW_LIB   = external/glfw-3.4/build/src/libglfw3.a

# Single OS tracking file for everything (GLFW + shaders + Vulkan headers)
BUILD_OS_FILE = .last_build_os

SHADER_DIR = shaders
VERT_SRC   = $(SHADER_DIR)/shader.vert
FRAG_SRC   = $(SHADER_DIR)/shader.frag
VERT_SPV   = $(SHADER_DIR)/vert.spv
FRAG_SPV   = $(SHADER_DIR)/frag.spv

# ── OS Detection ──────────────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
  PLATFORM          = windows
  TARGET            = $(PROJECT_NAME).exe
  DEL               = del /Q
  ZIPPER            = tar -a -c -f
  ARCHIVE_EXTENSION = zip
  ZIP_NAME          = $(PROJECT_NAME)_$(NAME).$(ARCHIVE_EXTENSION)
  VULKAN_SCRIPT     = powershell -ExecutionPolicy Bypass -File install_vulkan.ps1
  SHADER_SCRIPT     = compile_windows.bat

  LIBSTDCXX_PATH := $(shell g++ -print-file-name=libstdc++.a)
  LIBSTDCXX_DIR  := $(dir $(LIBSTDCXX_PATH))

  VULKAN_SDK_PATH ?= $(VULKAN_SDK)
  ifeq ($(VULKAN_SDK_PATH),)
    VULKAN_SDK_PATH = C:/VulkanSDK/1.4.341.1
  endif
  VULKAN_SDK_PATH := $(subst \,/,$(VULKAN_SDK_PATH))

  CXX_FLAGS += -I"$(VULKAN_SDK_PATH)/Include"
  LDFLAGS    = $(GLFW_LIB) \
               -L"$(VULKAN_SDK_PATH)/Lib" \
               -L"$(LIBSTDCXX_DIR)" \
               -lvulkan-1 \
               -lgdi32 -luser32 -lshell32 \
               -lstdc++

  GLFW_FLAGS = -D_GLFW_WIN32 -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src
  GLFW_SRCS  = \
    context.c init.c input.c monitor.c platform.c vulkan.c window.c \
    win32_init.c win32_joystick.c win32_module.c win32_monitor.c \
    win32_thread.c win32_time.c win32_window.c \
    wgl_context.c egl_context.c osmesa_context.c \
    null_init.c null_joystick.c null_monitor.c null_window.c

else
  PLATFORM          = linux
  TARGET            = $(PROJECT_NAME)
  DEL               = rm -f
  VULKAN_SCRIPT     = bash install_vulkan.sh
  SHADER_SCRIPT     = bash compile_linux.sh
  LDFLAGS           = $(GLFW_LIB) -lvulkan -ldl -lpthread

  ifneq ($(shell clang++ --version 2>/dev/null | grep -o "Target: x86_64"),)
    CXX = clang++
    CC  = clang
  endif

  ifneq ($(shell tar --version 2>/dev/null | grep -o "GNU tar"),)
    ARCHIVE_EXTENSION = tar.gz
    ZIPPER            = tar -acf
  else
    ARCHIVE_EXTENSION = zip
    ZIPPER            = zip
  endif

  ZIP_NAME = $(PROJECT_NAME)_$(NAME).$(ARCHIVE_EXTENSION)

  GLFW_FLAGS = -D_GLFW_X11 -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src
  GLFW_SRCS  = \
    context.c init.c input.c monitor.c platform.c vulkan.c window.c \
    x11_init.c x11_monitor.c x11_window.c \
    xkb_unicode.c \
    posix_module.c posix_poll.c posix_thread.c posix_time.c \
    glx_context.c egl_context.c osmesa_context.c \
    linux_joystick.c \
    null_init.c null_joystick.c null_monitor.c null_window.c

endif

GLFW_OBJS = $(addprefix $(GLFW_SRC)/,$(GLFW_SRCS:.c=.glfw.o))

# ── Targets ───────────────────────────────────────────────────────────────────
all: check-os-change check-vulkan compile-shaders $(TARGET)

# ── OS change detection — wipes everything if OS changed since last build ──────
check-os-change:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_OS_FILE)" ( \
		for /f "delims=" %%i in ($(BUILD_OS_FILE)) do ( \
			if not "%%i"=="$(PLATFORM)" ( \
				echo [OS] Platform changed from %%i to $(PLATFORM) - wiping all build artifacts... & \
				if exist "external\glfw-3.4\build\src\libglfw3.a" del /Q "external\glfw-3.4\build\src\libglfw3.a" & \
				for %%f in (external\glfw-3.4\src\*.glfw.o) do del /Q "%%f" & \
				if exist "shaders\vert.spv" del /Q "shaders\vert.spv" & \
				if exist "shaders\frag.spv" del /Q "shaders\frag.spv" & \
				for %%f in (*.o) do del /Q "%%f" & \
				if exist "main.exe" del /Q "main.exe" \
			) \
		) \
	)
	@echo $(PLATFORM)> "$(BUILD_OS_FILE)"
else
	@LAST=""; \
	if [ -f "$(BUILD_OS_FILE)" ]; then LAST=$$(cat "$(BUILD_OS_FILE)"); fi; \
	if [ -n "$$LAST" ] && [ "$$LAST" != "$(PLATFORM)" ]; then \
		echo "[OS] Platform changed from $$LAST to $(PLATFORM) - wiping all build artifacts..."; \
		rm -f "$(GLFW_LIB)" $(GLFW_SRC)/*.glfw.o; \
		rm -f "$(VERT_SPV)" "$(FRAG_SPV)"; \
		rm -f $(OBJECTS) $(TARGET); \
	fi; \
	echo "$(PLATFORM)" > "$(BUILD_OS_FILE)"
endif

# ── Vulkan check ──────────────────────────────────────────────────────────────
check-vulkan:
	@echo "[Makefile] Checking Vulkan installation ($(PLATFORM))..."
	@$(VULKAN_SCRIPT)

# ── Build GLFW ────────────────────────────────────────────────────────────────
$(GLFW_LIB): $(GLFW_OBJS)
ifeq ($(OS),Windows_NT)
	@if not exist "external\glfw-3.4\build\src" mkdir "external\glfw-3.4\build\src"
endif
ifeq ($(PLATFORM),linux)
	@mkdir -p external/glfw-3.4/build/src
endif
	@echo "[GLFW] Building libglfw3.a for $(PLATFORM)..."
	ar rcs $@ $^
	@echo "[GLFW] libglfw3.a built successfully."

$(GLFW_SRC)/%.glfw.o: $(GLFW_SRC)/%.c
	$(CC) -c $< -o $@ $(GLFW_FLAGS)

# ── Shader compilation ────────────────────────────────────────────────────────
compile-shaders: $(VERT_SPV) $(FRAG_SPV)

$(VERT_SPV): $(VERT_SRC)
	@echo "[Shaders] Compiling vertex shader..."
	@$(SHADER_SCRIPT)

$(FRAG_SPV): $(FRAG_SRC)
	@echo "[Shaders] Compiling fragment shader..."
	@$(SHADER_SCRIPT)

# ── Main build ────────────────────────────────────────────────────────────────
$(TARGET): $(OBJECTS) $(GLFW_LIB)
ifeq ($(OS),Windows_NT)
	-taskkill /F /IM $(TARGET) 2>nul
endif
	$(CXX) $(CPPVERSION) -o $@ $(OBJECTS) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXX_FLAGS) $(CPPVERSION) -o $@ -c $<

clean:
ifeq ($(OS),Windows_NT)
	-del /Q main.exe main.o shaders\vert.spv shaders\frag.spv 2>NUL
	-del /Q external\glfw-3.4\build\src\libglfw3.a 2>NUL
	-del /Q external\glfw-3.4\src\*.glfw.o 2>NUL
	-del /Q "$(BUILD_OS_FILE)" 2>NUL
	-cmd /c "del /F /Q \\?\$(CURDIR)\nul" 2>NUL
else
	$(DEL) $(TARGET) $(OBJECTS) $(VERT_SPV) $(FRAG_SPV)
	$(DEL) $(GLFW_LIB) $(GLFW_SRC)/*.glfw.o $(BUILD_OS_FILE)
endif

.PHONY: all clean depend submission check-vulkan check-os-change compile-shaders

depend:
	@sed -i.bak '/^# DEPENDENCIES/,$$d' Makefile
	@$(DEL) sed*
	@echo "# DEPENDENCIES" >> Makefile
	@$(CXX) $(CPPVERSION) -MM $(SOURCE) >> Makefile

submission:
	@echo "creating your submission as: $(ZIP_NAME) ..."
	@echo "...zipping source:    $(SOURCE) ..."
	@echo "...zipping headers:   $(H_FILES) ..."
	@echo "...zipping resources: $(RSR_FILES)..."
	@echo "...zipping Makefile ..."
	$(ZIPPER) $(ZIP_NAME) $(SOURCE) $(H_FILES) $(RSR_FILES) Makefile
	@echo "...$(ZIP_NAME) done check for errors"



# DEPENDENCIES
#TODO
