# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /home/jeanpierremartine/Documents/projets/navdata_pi

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jeanpierremartine/Documents/projets/navdata_pi/build

# Utility rule file for navdata-po-update.

# Include the progress variables for this target.
include CMakeFiles/navdata-po-update.dir/progress.make

CMakeFiles/navdata-po-update: ../po/navdata_pi.pot
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/jeanpierremartine/Documents/projets/navdata_pi/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "[navdata_pi]-po-update: Done."

navdata-po-update: CMakeFiles/navdata-po-update
navdata-po-update: CMakeFiles/navdata-po-update.dir/build.make

.PHONY : navdata-po-update

# Rule to build all files generated by this target.
CMakeFiles/navdata-po-update.dir/build: navdata-po-update

.PHONY : CMakeFiles/navdata-po-update.dir/build

CMakeFiles/navdata-po-update.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/navdata-po-update.dir/cmake_clean.cmake
.PHONY : CMakeFiles/navdata-po-update.dir/clean

CMakeFiles/navdata-po-update.dir/depend:
	cd /home/jeanpierremartine/Documents/projets/navdata_pi/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jeanpierremartine/Documents/projets/navdata_pi /home/jeanpierremartine/Documents/projets/navdata_pi /home/jeanpierremartine/Documents/projets/navdata_pi/build /home/jeanpierremartine/Documents/projets/navdata_pi/build /home/jeanpierremartine/Documents/projets/navdata_pi/build/CMakeFiles/navdata-po-update.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/navdata-po-update.dir/depend

