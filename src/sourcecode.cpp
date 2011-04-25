/*************************************************************************************************/
/**
	sourcecode.cpp

	Represents a piece of source code, whether from a file, or a macro definition.


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
#include <iomanip>
#include <sstream>

#include "sourcecode.h"
#include "asmexception.h"
#include "stringutils.h"
#include "globaldata.h"
#include "lineparser.h"
#include "symboltable.h"
#include "macro.h"

using namespace std;



/*************************************************************************************************/
/**
	SourceCode::SourceCode()

	Constructor for SourceCode

	@param		pFilename		Filename of source file to open

	The supplied file will be opened.  If there is a problem, an AsmException will be thrown.
*/
/*************************************************************************************************/
SourceCode::SourceCode( const string& filename, int lineNumber )
	:	m_forStackPtr( 0 ),
		m_ifStackPtr( 0 ),
		m_currentMacro( NULL ),
		m_filename( filename ),
		m_lineNumber( lineNumber ),
		m_lineStartPointer( 0 )
{
}



/*************************************************************************************************/
/**
	SourceCode::~SourceCode()

	Destructor for SourceCode

	The associated source file will be closed.
*/
/*************************************************************************************************/
SourceCode::~SourceCode()
{
}



/*************************************************************************************************/
/**
	SourceCode::Process()

	Process the associated source file
*/
/*************************************************************************************************/
void SourceCode::Process()
{
	// Remember the FOR and IF stack initial pointer values

	int initialForStackPtr = m_forStackPtr;
	int initialIfStackPtr = m_ifStackPtr;

	// Iterate through the file line-by-line

	string lineFromFile;

//	while ( getline( m_file, lineFromFile ) )
	while ( GetLine( lineFromFile ) )
	{
		// Convert tabs to spaces

		StringUtils::ExpandTabsToSpaces( lineFromFile, 8 );

//		// Display and process
//
//		if ( GlobalData::Instance().IsFirstPass() )
//		{
//			cout << setw( 5 ) << m_lineNumber << ": " << lineFromFile << endl;
//		}

		try
		{
			LineParser thisLine( this, lineFromFile );
			thisLine.Process();
		}
		catch ( AsmException_SyntaxError& e )
		{
			// Augment exception with more details
			e.SetFilename( m_filename );
			e.SetLineNumber( m_lineNumber );
			throw;
		}

		m_lineNumber++;
		m_lineStartPointer = GetFilePointer();
	}

	// Check whether we aborted prematurely

	if ( !IsAtEnd() )
	{
		throw AsmException_FileError_ReadSourceFile( m_filename );
	}

	// Check that we have no FOR / braces mismatch

	if ( m_forStackPtr > initialForStackPtr )
	{
		For& mismatchedFor = m_forStack[ m_forStackPtr - 1 ];

		if ( mismatchedFor.m_step == 0.0 )
		{
			AsmException_SyntaxError_MismatchedBraces e( mismatchedFor.m_line, mismatchedFor.m_column );
			e.SetFilename( m_filename );
			e.SetLineNumber( mismatchedFor.m_lineNumber );
			throw e;
		}
		else
		{
			AsmException_SyntaxError_ForWithoutNext e( mismatchedFor.m_line, mismatchedFor.m_column );
			e.SetFilename( m_filename );
			e.SetLineNumber( mismatchedFor.m_lineNumber );
			throw e;
		}
	}

	// Check that we have no IF / MACRO mismatch

	if ( m_ifStackPtr > initialIfStackPtr )
	{
		If& mismatchedIf = m_ifStack[ m_ifStackPtr - 1 ];

		if ( mismatchedIf.m_isMacroDefinition )
		{
			AsmException_SyntaxError_NoEndMacro e( mismatchedIf.m_line, mismatchedIf.m_column );
			e.SetFilename( m_filename );
			e.SetLineNumber( mismatchedIf.m_lineNumber );
			throw e;
		}
		else
		{
			AsmException_SyntaxError_IfWithoutEndif e( mismatchedIf.m_line, mismatchedIf.m_column );
			e.SetFilename( m_filename );
			e.SetLineNumber( mismatchedIf.m_lineNumber );
			throw e;
		}
	}
}



/*************************************************************************************************/
/**
	SourceCode::AddFor()
*/
/*************************************************************************************************/
void SourceCode::AddFor( const string& varName,
						 double start,
						 double end,
						 double step,
						 int filePtr,
						 const string& line,
						 int column )
{
	if ( m_forStackPtr == MAX_FOR_LEVELS )
	{
		throw AsmException_SyntaxError_TooManyFORs( line, column );
	}

	// Add symbol to table

	SymbolTable::Instance().AddSymbol( varName, start );

	// Fill in FOR block

	m_forStack[ m_forStackPtr ].m_varName		= varName;
	m_forStack[ m_forStackPtr ].m_current		= start;
	m_forStack[ m_forStackPtr ].m_end			= end;
	m_forStack[ m_forStackPtr ].m_step			= step;
	m_forStack[ m_forStackPtr ].m_filePtr		= filePtr;
	m_forStack[ m_forStackPtr ].m_id			= GlobalData::Instance().GetNextForId();
	m_forStack[ m_forStackPtr ].m_count			= 0;
	m_forStack[ m_forStackPtr ].m_line			= line;
	m_forStack[ m_forStackPtr ].m_column		= column;
	m_forStack[ m_forStackPtr ].m_lineNumber	= m_lineNumber;

	m_forStackPtr++;
}



/*************************************************************************************************/
/**
	SourceCode::OpenBrace()

	Braces for scoping variables are just FORs in disguise...
*/
/*************************************************************************************************/
void SourceCode::OpenBrace( const string& line, int column )
{
	if ( m_forStackPtr == MAX_FOR_LEVELS )
	{
		throw AsmException_SyntaxError_TooManyFORs( line, column );
	}

	// Fill in FOR block

	m_forStack[ m_forStackPtr ].m_varName		= "";
	m_forStack[ m_forStackPtr ].m_current		= 1.0;
	m_forStack[ m_forStackPtr ].m_end			= 0.0;
	m_forStack[ m_forStackPtr ].m_step			= 0.0;
	m_forStack[ m_forStackPtr ].m_filePtr		= 0;
	m_forStack[ m_forStackPtr ].m_id			= GlobalData::Instance().GetNextForId();
	m_forStack[ m_forStackPtr ].m_count			= 0;
	m_forStack[ m_forStackPtr ].m_line			= line;
	m_forStack[ m_forStackPtr ].m_column		= column;
	m_forStack[ m_forStackPtr ].m_lineNumber	= m_lineNumber;

	m_forStackPtr++;
}



/*************************************************************************************************/
/**
	SourceCode::UpdateFor()
*/
/*************************************************************************************************/
void SourceCode::UpdateFor( const string& line, int column )
{
	if ( m_forStackPtr == 0 )
	{
		throw AsmException_SyntaxError_NextWithoutFor( line, column );
	}

	For& thisFor = m_forStack[ m_forStackPtr - 1 ];

	// step of 0.0 here means that the 'for' is in fact an open brace, so throw an error

	if ( thisFor.m_step == 0.0 )
	{
		throw AsmException_SyntaxError_NextWithoutFor( line, column );
	}

	thisFor.m_current += thisFor.m_step;

	if ( ( thisFor.m_step > 0.0 && thisFor.m_current > thisFor.m_end ) ||
		 ( thisFor.m_step < 0.0 && thisFor.m_current < thisFor.m_end ) )
	{
		// we have reached the end of the FOR
		SymbolTable::Instance().RemoveSymbol( thisFor.m_varName );
		m_forStackPtr--;
	}
	else
	{
		// reloop
		SymbolTable::Instance().ChangeSymbol( thisFor.m_varName, thisFor.m_current );
		SetFilePointer( thisFor.m_filePtr );
		thisFor.m_count++;
		m_lineNumber = thisFor.m_lineNumber - 1;
	}
}



/*************************************************************************************************/
/**
	SourceCode::CloseBrace()

	Braces for scoping variables are just FORs in disguise...
*/
/*************************************************************************************************/
void SourceCode::CloseBrace( const string& line, int column )
{
	if ( m_forStackPtr == 0 )
	{
		throw AsmException_SyntaxError_MismatchedBraces( line, column );
	}

	For& thisFor = m_forStack[ m_forStackPtr - 1 ];

	// step of non-0.0 here means that this a real 'for', so throw an error

	if ( thisFor.m_step != 0.0 )
	{
		throw AsmException_SyntaxError_MismatchedBraces( line, column );
	}

	m_forStackPtr--;
}


/*************************************************************************************************/
/**
	SourceCode::CopyForStack()
*/
/*************************************************************************************************/
void SourceCode::CopyForStack( const SourceCode* copyFrom )
{
	m_forStackPtr = copyFrom->m_forStackPtr;

	for ( int i = 0; i < m_forStackPtr; i++ )
	{
		m_forStack[ i ] = copyFrom->m_forStack[ i ];
	}
}


/*************************************************************************************************/
/**
	SourceCode::GetSymbolNameSuffix()
*/
/*************************************************************************************************/
string SourceCode::GetSymbolNameSuffix( int level ) const
{
	if ( level == -1 )
	{
		level = m_forStackPtr;
	}

	ostringstream suffix;

	for ( int i = 0; i < level; i++ )
	{
		suffix << "@";
		suffix << m_forStack[ i ].m_id;
		suffix << "_";
		suffix << m_forStack[ i ].m_count;
	}

	return suffix.str();
}



/*************************************************************************************************/
/**
	SourceCode::IsIfConditionTrue()
*/
/*************************************************************************************************/
bool SourceCode::IsIfConditionTrue() const
{
	for ( int i = 0; i < m_ifStackPtr; i++ )
	{
		if ( !m_ifStack[ i ].m_condition )
		{
			return false;
		}
	}

	return true;
}



/*************************************************************************************************/
/**
	SourceCode::AddIfLevel()
*/
/*************************************************************************************************/
void SourceCode::AddIfLevel( const string& line, int column )
{
	if ( m_ifStackPtr == MAX_IF_LEVELS )
	{
		throw AsmException_SyntaxError_TooManyIFs( line, column );
	}

	m_ifStack[ m_ifStackPtr ].m_condition			= true;
	m_ifStack[ m_ifStackPtr ].m_passed				= false;
	m_ifStack[ m_ifStackPtr ].m_hadElse				= false;
	m_ifStack[ m_ifStackPtr ].m_isMacroDefinition	= false;
	m_ifStack[ m_ifStackPtr ].m_line				= line;
	m_ifStack[ m_ifStackPtr ].m_column				= column;
	m_ifStack[ m_ifStackPtr ].m_lineNumber			= m_lineNumber;
	m_ifStackPtr++;
}



/*************************************************************************************************/
/**
	SourceCode::SetCurrentIfAsMacroDefinition()
*/
/*************************************************************************************************/
void SourceCode::SetCurrentIfAsMacroDefinition()
{
	assert( m_ifStackPtr > 0 );
	m_ifStack[ m_ifStackPtr - 1 ].m_isMacroDefinition = true;
}



/*************************************************************************************************/
/**
	SourceCode::SetCurrentIfCondition()
*/
/*************************************************************************************************/
void SourceCode::SetCurrentIfCondition( bool b )
{
	assert( m_ifStackPtr > 0 );
	m_ifStack[ m_ifStackPtr - 1 ].m_condition = b;
	if ( b )
	{
		m_ifStack[ m_ifStackPtr - 1 ].m_passed = true;
	}
}



/*************************************************************************************************/
/**
	SourceCode::StartElse()
*/
/*************************************************************************************************/
void SourceCode::StartElse( const string& line, int column )
{
	if ( m_ifStack[ m_ifStackPtr - 1 ].m_hadElse )
	{
		throw AsmException_SyntaxError_ElseWithoutIf( line, column );
	}

	m_ifStack[ m_ifStackPtr - 1 ].m_hadElse = true;

	m_ifStack[ m_ifStackPtr - 1 ].m_condition = !m_ifStack[ m_ifStackPtr - 1 ].m_passed;
}



/*************************************************************************************************/
/**
	SourceCode::StartElif()
*/
/*************************************************************************************************/
void SourceCode::StartElif( const string& line, int column )
{
	if ( m_ifStack[ m_ifStackPtr - 1 ].m_hadElse )
	{
		throw AsmException_SyntaxError_ElifWithoutIf( line, column );
	}

	m_ifStack[ m_ifStackPtr - 1 ].m_condition = !m_ifStack[ m_ifStackPtr - 1 ].m_passed;
}



/*************************************************************************************************/
/**
	SourceCode::RemoveIfLevel()
*/
/*************************************************************************************************/
void SourceCode::RemoveIfLevel( const string& line, int column )
{
	if ( m_ifStackPtr == 0 )
	{
		throw AsmException_SyntaxError_EndifWithoutIf( line, column );
	}

	m_ifStackPtr--;
}



/*************************************************************************************************/
/**
	SourceCode::StartMacro()
*/
/*************************************************************************************************/
void SourceCode::StartMacro( const string& line, int column )
{
	if ( GlobalData::Instance().IsFirstPass() )
	{
		if ( m_currentMacro == NULL )
		{
			m_currentMacro = new Macro( m_filename, m_lineNumber );
		}
		else
		{
			throw AsmException_SyntaxError_NoNestedMacros( line, column );
		}
	}

	AddIfLevel( line, column );
	SetCurrentIfAsMacroDefinition();
}



/*************************************************************************************************/
/**
	SourceCode::EndMacro()
*/
/*************************************************************************************************/
void SourceCode::EndMacro( const string& line, int column )
{
	if ( GlobalData::Instance().IsFirstPass() &&
		 m_currentMacro == NULL )
	{
		throw AsmException_SyntaxError_EndMacroUnexpected( line, column - 8 );
	}

	RemoveIfLevel( line, column );

	if ( GlobalData::Instance().IsFirstPass() )
	{
//		cout << "Macro: '" << m_currentMacro->GetBody() << "'" << endl;
		MacroTable::Instance().Add( m_currentMacro );
		m_currentMacro = NULL;
	}
}
