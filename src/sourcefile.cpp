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

#include <fstream>
#include <iostream>

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

	Read a file into a string.  Convert tabs to spaces and
	normalise line endings (\r, \r\n or \n) to \n.

	@param		filename		Filename of source file to open

	The supplied file will be opened.  If there is a problem, an AsmException will be thrown.
*/
/*************************************************************************************************/
static string ReadFile( const string& filename )
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
	blob.reserve(length + 1); // Extra 1 for trailing '\n'

	std::istream::sentry sentry(file, true);
	std::streambuf* sb = file.rdbuf();

	while (true)
	{
		std::ifstream::int_type c = sb->sbumpc();
		if (c == '\t')
		{
			blob.push_back(' ');
		}
		else if (c == '\r')
		{
			std::ifstream::int_type next = sb->sgetc();
			if (next != '\n')
			{
				blob.push_back('\n');
			}
		}
		else if (c == std::ifstream::traits_type::eof())
		{
			break;
		}
		else
		{
			blob.push_back(c);
		}
	}
	if (blob.length() == 0 || blob[blob.length() - 1] != '\n')
	{
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
	:	SourceCode( filename, 1, ReadFile( filename ), parent )
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
