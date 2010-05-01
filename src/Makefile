#**************************************************************************************************
#
#	Makefile for beebasm
#
#**************************************************************************************************

# List source subdirectories

DIRS			:=

# Define object file path

BUILD_DIR		:=		./objects

# Define target file

TARGET			:=		../beebasm.exe

# Define compiler switches

WARNFLAGS		:=		-Wall -W -Wcast-qual -Werror -Wshadow -Wcast-align -Wold-style-cast -Woverloaded-virtual
CXXFLAGS		:=		-O3 -pedantic $(WARNFLAGS)

# Define linker switches

LDFLAGS			:=		-s

# Define 2nd party libs to link

LOADLIBES		:=

# Define GNU libs to link

LDLIBS			:=		-lstdc++

# Parameters to the executable

PARAMS			:=		-i ..\demo.asm -do ..\demo.ssd -boot Code



#--------------------------------------------------------------------------------------------------

include Makefile.inc
