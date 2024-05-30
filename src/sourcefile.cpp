/*************************************************************************************************/
/**
	sourcefile.cpp

	Assembles a file


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

#include <iostream>
#include <iomanip>
#include <sstream>

#include "sourcefile.h"
#include "asmexception.h"
#include "stringutils.h"
#include "globaldata.h"
#include "lineparser.h"
#include "symboltable.h"


using namespace std;


/*************************************************************************************************/
/**
	ReadFile()

	Read a file into a string

	@param		filename		Filename of source file to open

	The supplied file will be opened.  If there is a problem, an AsmException will be thrown.
*/
/*************************************************************************************************/
static string ReadFile( string filename )
{
	// we have to open in binary, due to a bug in MinGW which means that calling
	// tellg() on a text-mode file ruins the file pointer!
	// http://www.mingw.org/MinGWiki/index.php/Known%20Problems
	std::ifstream file;
	file.open( filename.c_str(), ios_base::binary );

	if ( !file )
	{
		throw AsmException_FileError_OpenSourceFile( filename );
	}

	file.seekg(0, std::ios_base::end);
	unsigned int length = static_cast<unsigned int>(file.tellg());
	file.seekg(0, std::ios_base::beg);

	string blob;
	blob.reserve(length + 2); // Extra 2 for trailing crlf

	string line;
	while (	getline( file, line ) )
	{
		blob.append(line);
		// This appends \r\n on Windows
		blob.append("\n");
	}

	return blob;
}

/*************************************************************************************************/
/**
	SourceFile::SourceFile()

	Constructor for SourceFile

	@param		filename		Filename of source file to open
	@param		parent			Parent SourceCode object

	The supplied file will be opened.  If there is a problem, an AsmException will be thrown.
*/
/*************************************************************************************************/
SourceFile::SourceFile( const string& filename, const SourceCode* parent )
	:	SourceCode( filename, 1, parent ),
		m_file( ReadFile( filename ) )
{
}



/*************************************************************************************************/
/**
	SourceFile::~SourceFile()

	Destructor for SourceFile

	The associated source file will be closed.
*/
/*************************************************************************************************/
SourceFile::~SourceFile()
{
}


/*************************************************************************************************/
/**
	SourceFile::Process()

	Process the associated source file
*/
/*************************************************************************************************/
void SourceFile::Process()
{
	SourceCode::Process();

	// Display ok message

	if ( ShouldOutputAsm() )
	{
		cerr << "Processed file '" << m_filename << "' ok" << endl;
	}
}



/*************************************************************************************************/
/**
	SourceFile::GetLine()

	Reads a line from the source code and returns it into lineFromFile
*/
/*************************************************************************************************/
istream& SourceFile::GetLine( string& lineFromFile ) 
{
	return getline( m_file, lineFromFile );
}



/*************************************************************************************************/
/**
	SourceFile::GetFilePointer()

	Returns the current file pointer
*/
/*************************************************************************************************/
int SourceFile::GetFilePointer()
{
	return static_cast< int >( m_file.tellg() );
}



/*************************************************************************************************/
/**
	SourceFile::SetFilePointer()

	Sets the current file pointer
*/
/*************************************************************************************************/
void SourceFile::SetFilePointer( int i )
{
	m_lineStartPointer = i;
	m_file.seekg( i );
}



/*************************************************************************************************/
/**
	SourceFile::IsAtEnd()

	Returns whether the current stream is in an end-of-file state
*/
/*************************************************************************************************/
bool SourceFile::IsAtEnd()
{
	return m_file.eof();
}
