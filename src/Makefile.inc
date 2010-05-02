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
#	@author camrtw
#
#**************************************************************************************************

# Check that required variables have been set

ifndef BUILD_DIR
error Please set BUILD_DIR to the path into which objects are compiled
endif

ifndef TARGET
error Please set TARGET to the filename of the final executable to be built
endif


# Get list of subdirectories

DIRS			:=		$(sort . $(DIRS))


# Automatically build list of source files

SRCS			:=		$(wildcard $(addsuffix /*.cpp,$(DIRS)) $(addsuffix /*.c,$(DIRS)))


# Get list of object/dep files from source files

OBJS			:=		$(subst /./,/,$(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS)))
DEPS			:=		$(OBJS:.o=.d)


# Declare executable names

CC				:=		C:/MinGW/bin/gcc
CXX				:=		C:/MinGW/bin/gcc
LD				:=		C:/MinGW/bin/gcc
MKDIR			:=		C:/Cygwin/bin/mkdir -p
RM				:=		C:/Cygwin/bin/rm -f
ECHO			:=		@@C:/Cygwin/bin/echo -e


# Declare default number of CPUs

CPUS			?=		1


# Set variables according to VERBOSE

ifdef VERBOSE
VB				:=
VB_MAKE			:=
else
VB				:=		@@
VB_MAKE			:=		-s
endif



#--------------------------------------------------------------------------------------------------
#	Rules/targets


.PHONY: folders all code deps objs run clean help


help:
	$(ECHO) Possible options:
	$(ECHO) make all .... Build and run code
	$(ECHO) make code ... Build code
	$(ECHO) make run .... Run code
	$(ECHO) make clean .. Clean code
	$(ECHO) make help ... Display this message again
	$(ECHO) Append VERBOSE=1 to show more information


folders: $(addprefix $(BUILD_DIR)/,$(DIRS))


objs: folders
	$(VB)$(MAKE) $(VB_MAKE) -j $(CPUS) $(OBJS) INCLUDE_DEPS=1


deps: folders
	$(VB)$(MAKE) $(VB_MAKE) -j $(CPUS) $(DEPS) INCLUDE_DEPS=1


code: deps objs $(TARGET)


run:
	$(ECHO) Running ... $(TARGET)\n
	$(VB)$(TARGET) $(PARAMS)


all: code run


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
