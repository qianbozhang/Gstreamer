# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid

# Include any dependencies generated for this target.
include bin/CMakeFiles/oggplayer.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/oggplayer.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/oggplayer.dir/flags.make

bin/CMakeFiles/oggplayer.dir/main.c.o: bin/CMakeFiles/oggplayer.dir/flags.make
bin/CMakeFiles/oggplayer.dir/main.c.o: ../src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object bin/CMakeFiles/oggplayer.dir/main.c.o"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/oggplayer.dir/main.c.o   -c /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src/main.c

bin/CMakeFiles/oggplayer.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/oggplayer.dir/main.c.i"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src/main.c > CMakeFiles/oggplayer.dir/main.c.i

bin/CMakeFiles/oggplayer.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/oggplayer.dir/main.c.s"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src/main.c -o CMakeFiles/oggplayer.dir/main.c.s

bin/CMakeFiles/oggplayer.dir/player.c.o: bin/CMakeFiles/oggplayer.dir/flags.make
bin/CMakeFiles/oggplayer.dir/player.c.o: ../src/player.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object bin/CMakeFiles/oggplayer.dir/player.c.o"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/oggplayer.dir/player.c.o   -c /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src/player.c

bin/CMakeFiles/oggplayer.dir/player.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/oggplayer.dir/player.c.i"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src/player.c > CMakeFiles/oggplayer.dir/player.c.i

bin/CMakeFiles/oggplayer.dir/player.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/oggplayer.dir/player.c.s"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src/player.c -o CMakeFiles/oggplayer.dir/player.c.s

# Object files for target oggplayer
oggplayer_OBJECTS = \
"CMakeFiles/oggplayer.dir/main.c.o" \
"CMakeFiles/oggplayer.dir/player.c.o"

# External object files for target oggplayer
oggplayer_EXTERNAL_OBJECTS =

../output/oggplayer: bin/CMakeFiles/oggplayer.dir/main.c.o
../output/oggplayer: bin/CMakeFiles/oggplayer.dir/player.c.o
../output/oggplayer: bin/CMakeFiles/oggplayer.dir/build.make
../output/oggplayer: bin/CMakeFiles/oggplayer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable ../../output/oggplayer"
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/oggplayer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/oggplayer.dir/build: ../output/oggplayer

.PHONY : bin/CMakeFiles/oggplayer.dir/build

bin/CMakeFiles/oggplayer.dir/clean:
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin && $(CMAKE_COMMAND) -P CMakeFiles/oggplayer.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/oggplayer.dir/clean

bin/CMakeFiles/oggplayer.dir/depend:
	cd /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/src /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin /home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/bulid/bin/CMakeFiles/oggplayer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/oggplayer.dir/depend
