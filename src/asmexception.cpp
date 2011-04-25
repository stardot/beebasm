/*************************************************************************************************/
/**
	asmexception.cpp

	Exception handling for the app


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

#include <iostream>
#include <cassert>

#include "asmexception.h"
#include "stringutils.h"

using namespace std;



/*************************************************************************************************/
/**
	AsmException_FileAccessError::Print()

	Outputs to stderr an error message relating to an I/O exception
*/
/*************************************************************************************************/
void AsmException_FileError::Print() const
{
	cerr << "Error: " << m_filename << ": " << Message() << endl;
}



/*************************************************************************************************/
/**
	AsmException_SyntaxError::Print()

	Outputs to stderr an error message regarding a syntax error
*/
/*************************************************************************************************/
void AsmException_SyntaxError::Print() const
{
	assert( m_filename != "" );
	assert( m_lineNumber != 0 );

	cerr << m_filename << ":" << m_lineNumber << ": error: ";
	cerr << Message() << endl << endl;
	cerr << m_line << endl;
	cerr << string( m_column, ' ' ) << "^" << endl;
}

