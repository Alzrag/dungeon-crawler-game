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


CXX_FLAGS = -Wall -Wextra -Wconversion -Wdouble-promotion -Wunreachable-code -Wshadow -Wpedantic
LDFLAGS = $(shell pkg-config --libs glfw3) -lvulkan -ldl -lpthread
CPPVERSION = -std=c++17

OBJECTS = $(SOURCE:.cpp=.o)

ifeq ($(shell echo "Windows"), "Windows")
	TARGET = $(PROJECT_NAME).exe
	DEL = del
	ZIPPER = tar -a -c -f
  ARCHIVE_EXTENSION = zip
	ZIP_NAME = $(PROJECT_NAME)_$(NAME).$(ARCHIVE_EXTENSION)

else
	TARGET = $(PROJECT_NAME)
	DEL = rm -f
	ZIPPER = tar -acf

	ifeq ($(shell tar --version | grep -o "GNU tar"), GNU tar)
	ARCHIVE_EXTENSION = tar.gz
	endif

	ifeq ($(shell clang++ --version | grep -o "Target: x86_64"), Target: x86_64)
	CXX=clang++
	endif

	ZIP_NAME = $(PROJECT_NAME)_$(NAME).$(ARCHIVE_EXTENSION)

endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CPPVERSION) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXX_FLAGS) $(CPPVERSION) -o $@ -c $<

clean:
	$(DEL) $(TARGET) $(OBJECTS)

depend:
	@sed -i.bak '/^# DEPENDENCIES/,$$d' Makefile
	@$(DEL) sed*
	@echo "# DEPENDENCIES" >> Makefile
	@$(CXX) $(CPPVERSION) -MM $(SOURCE) >> Makefile

submission:
	@echo "creating your submission as: $(ZIP_NAME) ..."
	@echo "...zipipping source:   $(SOURCE) ..."
	@echo "...zipping headers:   $(H_FILES) ..."
	@echo "...zipping resources: $(RSR_FILES)..."
	@echo "...zipping Makefile ..."
	$(ZIPPER) $(ZIP_NAME) $(SOURCE) $(H_FILES) $(RSR_FILES) Makefile
	@echo "...$(ZIP_NAME) done check for errors"

.PHONY: all clean depend submission

# DEPENDENCIES
main.o: main.cpp

