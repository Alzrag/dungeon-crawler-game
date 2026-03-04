#project name
PROJECT_NAME=main
#source file location
SOURCE= main.cpp first_app.cpp lve_window.cpp
#any additional header files you need
H_FILES = first_app.hpp lve_window.hpp
#any additional resorce files you need
RSR_FILES = secretMessage.txt
#your name
NAME=Clark_Wallace

CXX = g++
CC  = gcc
CXX_FLAGS = -Wall -Wextra -Wconversion -Wdouble-promotion -Wunreachable-code -Wshadow -Wpedantic \
            -Iexternal/glfw-3.4/include -Iexternal/Vulkan-Headers/include
CPPVERSION = -std=c++17
OBJECTS = $(SOURCE:.cpp=.o)

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

  GLFW_SRC     = external/glfw-3.4/src
  GLFW_LIB     = external/glfw-3.4/build/src/libglfw3.a

  CXX_FLAGS += -I"$(VULKAN_SDK_PATH)/Include"
  LDFLAGS    = $(GLFW_LIB) \
               -L"$(VULKAN_SDK_PATH)/Lib" \
               -lvulkan-1 \
               -lgdi32 -luser32 -lshell32

else
  PLATFORM          = linux
  TARGET            = $(PROJECT_NAME)
  DEL               = rm -f
  VULKAN_SCRIPT     = bash install_vulkan.sh
  GLFW_LIB          = external/glfw-3.4/build/src/libglfw3.a
  LDFLAGS           = $(GLFW_LIB) -lvulkan -ldl -lpthread

  ifneq ($(shell clang++ --version 2>/dev/null | grep -o "Target: x86_64"),)
    CXX = clang++
  endif

  ifneq ($(shell tar --version 2>/dev/null | grep -o "GNU tar"),)
    ARCHIVE_EXTENSION = tar.gz
    ZIPPER            = tar -acf
  else
    ARCHIVE_EXTENSION = zip
    ZIPPER            = zip
  endif

  ZIP_NAME = $(PROJECT_NAME)_$(NAME).$(ARCHIVE_EXTENSION)
endif

# ── Targets ───────────────────────────────────────────────────────────────────
all: check-vulkan build-glfw $(TARGET)

# ── Vulkan check ──────────────────────────────────────────────────────────────
check-vulkan:
	@echo "[Makefile] Checking Vulkan installation ($(PLATFORM))..."
	@$(VULKAN_SCRIPT)

# ── Build GLFW from source (Windows only) ────────────────────────────────────
# Uses a shell existence check so GLFW is only ever compiled once.
# Run 'make clean' to force a rebuild.
ifeq ($(OS),Windows_NT)
build-glfw:
	@if exist "external\glfw-3.4\build\src\libglfw3.a" \
		( echo [GLFW] libglfw3.a already exists, skipping. ) \
	else \
		( echo [GLFW] Building libglfw3.a from source... \
		& if not exist "external\glfw-3.4\build\src" mkdir "external\glfw-3.4\build\src" \
		& $(CC) -c $(GLFW_SRC)/context.c      -o $(GLFW_SRC)/context.glfw.o      -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/init.c         -o $(GLFW_SRC)/init.glfw.o         -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/input.c        -o $(GLFW_SRC)/input.glfw.o        -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/monitor.c      -o $(GLFW_SRC)/monitor.glfw.o      -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/platform.c     -o $(GLFW_SRC)/platform.glfw.o     -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/vulkan.c       -o $(GLFW_SRC)/vulkan.glfw.o       -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/window.c       -o $(GLFW_SRC)/window.glfw.o       -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_init.c   -o $(GLFW_SRC)/win32_init.glfw.o   -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_joystick.c -o $(GLFW_SRC)/win32_joystick.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_module.c -o $(GLFW_SRC)/win32_module.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_monitor.c -o $(GLFW_SRC)/win32_monitor.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_thread.c -o $(GLFW_SRC)/win32_thread.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_time.c   -o $(GLFW_SRC)/win32_time.glfw.o   -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/win32_window.c -o $(GLFW_SRC)/win32_window.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/wgl_context.c  -o $(GLFW_SRC)/wgl_context.glfw.o  -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/egl_context.c  -o $(GLFW_SRC)/egl_context.glfw.o  -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/osmesa_context.c -o $(GLFW_SRC)/osmesa_context.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/null_init.c    -o $(GLFW_SRC)/null_init.glfw.o    -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/null_joystick.c -o $(GLFW_SRC)/null_joystick.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/null_monitor.c -o $(GLFW_SRC)/null_monitor.glfw.o -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& $(CC) -c $(GLFW_SRC)/null_window.c  -o $(GLFW_SRC)/null_window.glfw.o  -Iexternal/glfw-3.4/include -Iexternal/glfw-3.4/src -D_GLFW_WIN32 \
		& ar rcs $(GLFW_LIB) $(GLFW_SRC)/*.glfw.o \
		& echo [GLFW] libglfw3.a built successfully. )
else
build-glfw:
	@echo "[GLFW] Skipping GLFW build on Linux (use cmake or package manager)."
endif

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