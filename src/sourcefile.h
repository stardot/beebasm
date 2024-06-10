/*************************************************************************************************/
/**
	sourcefile.h


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

#ifndef SOURCEFILE_H_
#define SOURCEFILE_H_

#include <string>
#include <vector>
#include "sourcecode.h"
#include "macro.h"


class SourceFile : public SourceCode
{
public:

	// Constructor/destructor (RAII class)

	SourceFile( const std::string& filename, const SourceCode* parent );
	virtual ~SourceFile();

	virtual void Process();
};


#endif // SOURCEFILE_H_
