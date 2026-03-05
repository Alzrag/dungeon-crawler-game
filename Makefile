#project name
PROJECT_NAME=main
#source file location
SOURCE= main.cpp first_app.cpp lve_window.cpp lve_pipeline.cpp
#any additional header files you need
H_FILES = first_app.hpp lve_window.hpp lve_pipeline.hpp
#any additional resorce files you need
RSR_FILES = secretMessage.txt
#your name
NAME=Clark_Wallace

CXX = g++
CC  = gcc
CXX_FLAGS = -Wall -Wextra -Wconversion -Wdouble-promotion -Wunreachable-code -Wshadow -Wpedantic \
            -Iexternal/glfw-3.4/include -Iexternal/Vulkan-Headers/include
CPPVERSION = -std=c++17
OBJECTS    = $(SOURCE:.cpp=.o)
GLFW_SRC   = external/glfw-3.4/src
GLFW_LIB   = external/glfw-3.4/build/src/libglfw3.a
GLFW_OS_FILE = external/glfw-3.4/build/src/last_build_os.txt

# ── OS Detection ──────────────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
  PLATFORM          = windows
  TARGET            = $(PROJECT_NAME).exe
  DEL               = del /Q
  ZIPPER            = tar -a -c -f
  ARCHIVE_EXTENSION = zip
  ZIP_NAME          = $(PROJECT_NAME)_$(NAME).$(ARCHIVE_EXTENSION)
  VULKAN_SCRIPT     = powershell -ExecutionPolicy Bypass -File install_vulkan.ps1

  VULKAN_SDK_PATH ?= $(VULKAN_SDK)
  ifeq ($(VULKAN_SDK_PATH),)
    VULKAN_SDK_PATH = C:/VulkanSDK/1.4.341.1
  endif
  VULKAN_SDK_PATH := $(subst \,/,$(VULKAN_SDK_PATH))

  CXX_FLAGS += -I"$(VULKAN_SDK_PATH)/Include"
  LDFLAGS    = $(GLFW_LIB) \
               -L"$(VULKAN_SDK_PATH)/Lib" \
               -lvulkan-1 \
               -lgdi32 -luser32 -lshell32

  # Windows GLFW sources (Win32 backend)
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

  # Linux GLFW sources (X11 backend)
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
all: check-vulkan build-glfw $(TARGET)

# ── Vulkan check ──────────────────────────────────────────────────────────────
check-vulkan:
	@echo "[Makefile] Checking Vulkan installation ($(PLATFORM))..."
	@$(VULKAN_SCRIPT)

# ── Build GLFW (auto-rebuilds if OS changed since last build) ─────────────────
build-glfw:
ifeq ($(OS),Windows_NT)
	@if exist "external\glfw-3.4\build\src\last_build_os.txt" ( \
		for /f "delims=" %%i in (external\glfw-3.4\build\src\last_build_os.txt) do \
		if not "%%i"=="windows" ( \
			echo [GLFW] OS changed from %%i to windows - rebuilding GLFW... & \
			del /Q "external\glfw-3.4\build\src\libglfw3.a" 2>nul & \
			del /Q "external\glfw-3.4\src\*.glfw.o" 2>nul \
		) \
	)
	@if exist "external\glfw-3.4\build\src\libglfw3.a" ( \
		echo [GLFW] libglfw3.a already exists, skipping. \
	) else ( \
		echo [GLFW] Building libglfw3.a for windows... & \
		if not exist "external\glfw-3.4\build\src" mkdir "external\glfw-3.4\build\src" & \
		$(MAKE) $(GLFW_LIB) & \
		echo windows> "external\glfw-3.4\build\src\last_build_os.txt" \
	)
else
	@LAST=""; \
	if [ -f "$(GLFW_OS_FILE)" ]; then LAST=$$(cat "$(GLFW_OS_FILE)"); fi; \
	if [ -n "$$LAST" ] && [ "$$LAST" != "$(PLATFORM)" ]; then \
		echo "[GLFW] OS changed from $$LAST to $(PLATFORM) - rebuilding GLFW..."; \
		rm -f "$(GLFW_LIB)" $(GLFW_SRC)/*.glfw.o; \
	fi
	@if [ -f "$(GLFW_LIB)" ]; then \
		echo "[GLFW] libglfw3.a already exists, skipping."; \
	else \
		echo "[GLFW] Building libglfw3.a for $(PLATFORM)..."; \
		mkdir -p external/glfw-3.4/build/src; \
		$(MAKE) $(GLFW_LIB); \
		echo "$(PLATFORM)" > "$(GLFW_OS_FILE)"; \
	fi
endif

$(GLFW_LIB): $(GLFW_OBJS)
	ar rcs $@ $^
	@echo "[GLFW] libglfw3.a built successfully."

$(GLFW_SRC)/%.glfw.o: $(GLFW_SRC)/%.c
	$(CC) -c $< -o $@ $(GLFW_FLAGS)

# ── Main build ────────────────────────────────────────────────────────────────
$(TARGET): $(OBJECTS)
ifeq ($(OS),Windows_NT)
	-taskkill /F /IM $(TARGET) 2>nul
endif
	$(CXX) $(CPPVERSION) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXX_FLAGS) $(CPPVERSION) -o $@ -c $<

clean:
ifeq ($(OS),Windows_NT)
	-$(DEL) $(TARGET) $(OBJECTS) 2>nul
else
	$(DEL) $(TARGET) $(OBJECTS)
endif

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

.PHONY: all clean depend submission check-vulkan build-glfw

# DEPENDENCIES
main.o: main.cpp
