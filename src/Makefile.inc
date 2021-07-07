#**************************************************************************************************
#
#	Standard Makefile core v1.0
#
#	This builds all .cpp and .c files it finds within the current directory and any specified
#	subdirectories, handling dependencies automatically, and outputs the specified executable.
#
#	This requires the following to be defined:
#
#		BUILD_DIR		Path into which objects are compiled
#		TARGET			Name of final executable
#
#	Additionally the following may be defined:
#
#		DIRS			Any subdirectories containing additional source files
#		CFLAGS			Any flags to be passed to the C compiler
#		CXXFLAGS		Any flags to be passed to the C++ compiler
#		LDFLAGS			Any flags to be passed to the linker
#		LOADLIBES		Any 2nd party libraries to be linked with
#		LDLIBS			Any GNU libraries to be linked with
#		CPUS			The number of CPUs on this machine
#		VERBOSE			Echoes all commands launched by make
#
#
#	Copyright (C) Rich Talbot-Watkins 2007-2012
#
#	This file is part of BeebAsm.
#
#	BeebAsm is free software: you can redistribute it and/or modify it under the terms of the GNU
#	General Public License as published by the Free Software Foundation, either version 3 of the
#	License, or (at your option) any later version.
#
#	BeebAsm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
#	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License along with BeebAsm, as
#	COPYING.txt.  If not, see <http://www.gnu.org/licenses/>.
#
#**************************************************************************************************

# Check that required variables have been set

ifndef BUILD_DIR
$(error Please set BUILD_DIR to the path into which objects are compiled)
endif

ifndef TARGET
$(error Please set TARGET to the filename of the final executable to be built)
endif


# Declare executable names

CC				:=		gcc
CXX				:=		g++
LD				:=		g++
MKDIR			:=		mkdir -p
RM				:=		rm -f
ECHO			:=		@@echo -e


# List of allowed platform names.
# Add extra valid platforms to this list as they are supported.

PLATFORM_LIST	:=		mingw-gcc
PLATFORM_LIST	+=		linux
PLATFORM_LIST	+=		macosx


# Try to determine the build platform if it wasn't explicitly specified on the command line

ifndef PLATFORM
# Do checks with $(shell uname -s) and/or $(OSTYPE) here to try to automatically determine
# the appropriate platform type.  Ultimately, the goal is to automatically set the make
# variable $(PLATFORM) to one of the values in PLATFORM_LIST, above.

ifneq (,$(findstring linux,$(OSTYPE)))
PLATFORM		:=		linux
else ifeq ($(shell uname -s),Linux)
PLATFORM		:=		linux
endif

ifneq (,$(findstring darwin,$(OSTYPE)))
PLATFORM		:=		macosx
else ifeq ($(shell uname -s),Darwin)
PLATFORM		:=		macosx
endif

ifeq ($(OS),Windows_NT)
PLATFORM		:=		mingw-gcc
endif

ifdef PLATFORM
ifndef INCLUDE_DEPS
$(info Automatically using platform $(PLATFORM))
endif
endif

endif


# If we still haven't got a valid value for $(PLATFORM), exit gracefully with a helpful message

ifeq ($(findstring $(PLATFORM),$(PLATFORM_LIST)),)
ifneq ($(MAKECMDGOALS),)
ifneq ($(MAKECMDGOALS),help)
$(error Could not determine the target platform, or an invalid value for PLATFORM was specified. Type 'make help' for information)
endif
endif
endif


# Get list of subdirectories

DIRS			:=		$(sort . $(DIRS))


# Automatically build list of source files

SRCS			:=		$(wildcard $(addsuffix /*.cpp,$(DIRS)) $(addsuffix /*.c,$(DIRS)))


# Get list of object/dep files from source files

OBJS			:=		$(subst /./,/,$(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS)))
DEPS			:=		$(OBJS:.o=.d)


# Add platform-specific flags

ifeq ($(PLATFORM),mingw-gcc)
LDFLAGS			+=		-static
TARGET			:=		$(addsuffix .exe,$(TARGET))
endif


# Declare default number of CPUs

CPUS			?=		1


# Set variables according to VERBOSE

ifdef VERBOSE
VB				:=
VB_MAKE			:=
VB_TEST			:=	-v
else
VB				:=		@@
VB_MAKE			:=		-s
VB_TEST			:=
endif



#--------------------------------------------------------------------------------------------------
#	Rules/targets


.PHONY: folders all code deps objs run clean help


help:
	$(ECHO) BeebAsm makefile
	$(ECHO) ""
	$(ECHO) "Syntax: make <option> [PLATFORM=<platform>] [VERBOSE=1]"
	$(ECHO) ""
	$(ECHO) Possible options:
	$(ECHO) make all .... Build and run code
	$(ECHO) make code ... Build code
	$(ECHO) make run .... Run code
	$(ECHO) make test ... Run tests
	$(ECHO) make clean .. Clean code
	$(ECHO) make help ... Display this message again
	$(ECHO)
	$(ECHO) Possible platforms: $(PLATFORM_LIST)
	$(ECHO)
	$(ECHO) Append VERBOSE=1 to show more information


folders: $(addprefix $(BUILD_DIR)/,$(DIRS))


objs: folders
	$(VB)$(MAKE) $(VB_MAKE) -j $(CPUS) $(OBJS) INCLUDE_DEPS=1

deps: folders
	$(VB)$(MAKE) $(VB_MAKE) -j $(CPUS) $(DEPS) INCLUDE_DEPS=1

code: deps objs $(TARGET)

run:
	$(ECHO) Running ... $(TARGET)
	$(VB)$(TARGET) $(PARAMS)


test:
	$(VB)$(TEST) $(VB_TEST)


all: code run test


clean:
	$(ECHO) Cleaning target and objects...
	$(VB)$(RM) $(TARGET)
	$(VB)$(RM) -r $(BUILD_DIR)



#--------------------------------------------------------------------------------------------------
#	Rules/targets


# Build dep file from C++ file

$(BUILD_DIR)/%.d : %.cpp
	$(ECHO) Generating dependencies for ... $<
	$(VB)$(CXX) $(CXXFLAGS) -MM -MF $@ -MT $@ -MT $(@:.d=.o) -c $<


# Build dep file from C file

$(BUILD_DIR)/%.d : %.c
	$(ECHO) Generating dependencies for ... $<
	$(VB)$(CC) $(CFLAGS) -MM -MF $@ -MT $@ -MT $(@:.d=.o) -c $<


# Build object from C++ file

$(BUILD_DIR)/%.o : %.cpp
	$(ECHO) Compiling ... $<
	$(VB)$(CXX) $(CXXFLAGS) -c $< -o $@
ifdef OUTPUT_SOURCE
	$(VB)$(CXX) $(CXXFLAGS) -S $< -o $(@:.o=.s)
endif


# Build object from C file

$(BUILD_DIR)/%.o : %.c
	$(ECHO) Compiling ... $<
	$(VB)$(CC) $(CFLAGS) -c $< -o $@
ifdef OUTPUT_SOURCE
	$(VB)$(CXX) $(CXXFLAGS) -S $< -o $(@:.o=.s)
endif


# Build target

$(TARGET) : $(OBJS)
	$(ECHO) Linking ... $@
	$(VB)$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LOADLIBES) $(LDLIBS)


# Create object subdirectory

$(addprefix $(BUILD_DIR)/,$(DIRS)):
	$(ECHO) Making subdirectory ... $@
	$(VB)$(MKDIR) $@


# Include dependencies

ifdef INCLUDE_DEPS
-include $(DEPS)
endif
