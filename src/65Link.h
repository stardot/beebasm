/*************************************************************************************************/
/**
	65Link.h

	Saves file(s) to 65Link volume.


	Copyright (C) Rich Talbot-Watkins 2007 - 2012

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

#ifndef H65LINK_H_
#define H65LINK_H_

// Saves 65Link file - data (no extension) and .lea.
//
// pVolumeName is path of 65Link drive, e.g.,
// "/home/tom/beeb/files/work/0".
//
// pBBCName is the name of the BBC file, e.g., "D.TEST". If the file
// appears to have no directory, $ is assumed.
//
// pAddr, len defines data to save.
//
// load, exec are the usual attributes.
void Save65LinkFile( const char *pVolumeName,
					 const char *pBBCName,
					 const unsigned char *pAddr,
					 int load,
					 int exec,
					 int len );

#endif
