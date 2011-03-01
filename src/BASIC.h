/*************************************************************************************************/
/**
	BASIC.h

	Contains routines for tokenising/detokenising BBC BASIC programs.

	Modified from code by Thomas Harte.

	Copyright (C) Thomas Harte

	This file is part of BeebAsm.

	BeebAsm is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	BeebAsm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with BeebAsm, as
	COPYING.txt.  If not, see <http://www.gnu.org/licenses/>.
*/
/*************************************************************************************************/

#ifndef BASIC_H_
#define BASIC_H_

typedef unsigned char Uint8;
typedef unsigned short Uint16;

void SetupBASICTables();
const char *GetBASICError();
int GetBASICErrorNum();
bool ExportBASIC(const char *Filename, Uint8 *Memory);
bool ImportBASIC(const char *Filename, Uint8 *Mem, int* Size);


#endif // BASIC_H_
