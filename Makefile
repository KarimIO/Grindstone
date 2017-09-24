# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/karimah/Work/Grindstone

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/karimah/Work/Grindstone

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target test
test:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running tests..."
	/usr/bin/ctest --force-new-ctest-process $(ARGS)
.PHONY : test

# Special rule for the target test
test/fast: test

.PHONY : test/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/cmake-gui -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/karimah/Work/Grindstone/CMakeFiles /home/karimah/Work/Grindstone/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/karimah/Work/Grindstone/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named Grindstone

# Build rule for target.
Grindstone: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 Grindstone
.PHONY : Grindstone

# fast build rule for target.
Grindstone/fast:
	$(MAKE) -f sources/code/Engine/CMakeFiles/Grindstone.dir/build.make sources/code/Engine/CMakeFiles/Grindstone.dir/build
.PHONY : Grindstone/fast

#=============================================================================
# Target rules for targets named converter

# Build rule for target.
converter: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 converter
.PHONY : converter

# fast build rule for target.
converter/fast:
	$(MAKE) -f sources/code/Converter/CMakeFiles/converter.dir/build.make sources/code/Converter/CMakeFiles/converter.dir/build
.PHONY : converter/fast

#=============================================================================
# Target rules for targets named converterDDS

# Build rule for target.
converterDDS: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 converterDDS
.PHONY : converterDDS

# fast build rule for target.
converterDDS/fast:
	$(MAKE) -f sources/code/ConverterDDS/CMakeFiles/converterDDS.dir/build.make sources/code/ConverterDDS/CMakeFiles/converterDDS.dir/build
.PHONY : converterDDS/fast

#=============================================================================
# Target rules for targets named graphicsgl

# Build rule for target.
graphicsgl: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 graphicsgl
.PHONY : graphicsgl

# fast build rule for target.
graphicsgl/fast:
	$(MAKE) -f sources/code/GraphicsOpenGL/CMakeFiles/graphicsgl.dir/build.make sources/code/GraphicsOpenGL/CMakeFiles/graphicsgl.dir/build
.PHONY : graphicsgl/fast

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... test"
	@echo "... edit_cache"
	@echo "... Grindstone"
	@echo "... converter"
	@echo "... converterDDS"
	@echo "... graphicsgl"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system
