# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.4/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build

# Include any dependencies generated for this target.
include CMakeFiles/integrate.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/integrate.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/integrate.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/integrate.dir/flags.make

CMakeFiles/integrate.dir/integral.cpp.o: CMakeFiles/integrate.dir/flags.make
CMakeFiles/integrate.dir/integral.cpp.o: /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/integral.cpp
CMakeFiles/integrate.dir/integral.cpp.o: CMakeFiles/integrate.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/integrate.dir/integral.cpp.o"
	/opt/homebrew/opt/llvm@17/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/integrate.dir/integral.cpp.o -MF CMakeFiles/integrate.dir/integral.cpp.o.d -o CMakeFiles/integrate.dir/integral.cpp.o -c /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/integral.cpp

CMakeFiles/integrate.dir/integral.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/integrate.dir/integral.cpp.i"
	/opt/homebrew/opt/llvm@17/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/integral.cpp > CMakeFiles/integrate.dir/integral.cpp.i

CMakeFiles/integrate.dir/integral.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/integrate.dir/integral.cpp.s"
	/opt/homebrew/opt/llvm@17/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/integral.cpp -o CMakeFiles/integrate.dir/integral.cpp.s

# Object files for target integrate
integrate_OBJECTS = \
"CMakeFiles/integrate.dir/integral.cpp.o"

# External object files for target integrate
integrate_EXTERNAL_OBJECTS =

integrate: CMakeFiles/integrate.dir/integral.cpp.o
integrate: CMakeFiles/integrate.dir/build.make
integrate: benchmark/src/libbenchmark.a
integrate: CMakeFiles/integrate.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable integrate"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/integrate.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/integrate.dir/build: integrate
.PHONY : CMakeFiles/integrate.dir/build

CMakeFiles/integrate.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/integrate.dir/cmake_clean.cmake
.PHONY : CMakeFiles/integrate.dir/clean

CMakeFiles/integrate.dir/depend:
	cd /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build /Users/diegofonseca/Desktop/GitRepos/Feeshy-Integral/build/CMakeFiles/integrate.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/integrate.dir/depend

