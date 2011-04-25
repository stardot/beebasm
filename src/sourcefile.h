/*************************************************************************************************/
/**
	sourcefile.h


	Copyright (C) Rich Talbot-Watkins 2007 - 2011

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

#include <fstream>
#include <string>
#include <vector>
#include "sourcecode.h"
#include "macro.h"


class SourceFile : public SourceCode
{
public:

	// Constructor/destructor (RAII class)

	explicit SourceFile( const std::string& filename );
	virtual ~SourceFile();

	virtual void Process();


	// Accessors

	virtual int				GetFilePointer();
	virtual void			SetFilePointer( int i );
	virtual std::istream&	GetLine( std::string& lineFromFile );
	virtual bool			IsAtEnd();


private:

	std::ifstream			m_file;
};


#endif // SOURCEFILE_H_
