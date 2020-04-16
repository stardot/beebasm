/*************************************************************************************************/
/**
	constants.h


	Copyright (C) 2018

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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// openbsd's M_PI contains a C-style cast, which clashes with gcc's -Wold-style-cast
const double const_pi = 3.14159265358979323846;

#endif // CONSTANTS_H_
