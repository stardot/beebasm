#**************************************************************************************************
#
#	Makefile for beebasm
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

# List source subdirectories

DIRS			:=

# Define object file path

BUILD_DIR		:=		./objects

# Define target file (minus any platform-specific extension)

TARGET			:=		../beebasm

# Define compiler switches

WARNFLAGS		:=		-Wall -W -Wcast-qual -Werror -Wshadow -Wcast-align -Wold-style-cast -Woverloaded-virtual
CXXFLAGS		:=		-O3 -pedantic -DNDEBUG $(WARNFLAGS)

# Define linker switches

LDFLAGS			:=		-s

# Define 2nd party libs to link

LOADLIBES		:=

# Define GNU libs to link

LDLIBS			:=		-lm

# Parameters to the executable

PARAMS			:=		-i ../demo.6502 -do ../demo.ssd -boot Code -v

# Command to run the tests

TEST			:=		cd .. && python3 test/testrunner.py


#--------------------------------------------------------------------------------------------------

include Makefile.inc
