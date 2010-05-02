/*************************************************************************************************/
/**
	sourcefile.cpp

	Assembles a file
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
	SourceFile::SourceFile()

	Constructor for SourceFile

	@param		pFilename		Filename of source file to open

	The supplied file will be opened.  If there is a problem, an AsmException will be thrown.
*/
/*************************************************************************************************/
SourceFile::SourceFile( const char* pFilename )
	:	m_pFilename( pFilename ),
		m_lineNumber( 1 ),
		m_filePointer( 0 )
{
	// we have to open in binary, due to a bug in MinGW which means that calling
	// tellg() on a text-mode file ruins the file pointer!
	// http://www.mingw.org/MinGWiki/index.php/Known%20Problems
	m_file.open( pFilename, ios_base::binary );

	if ( !m_file )
	{
		throw AsmException_FileError_OpenSourceFile( pFilename );
	}
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
	m_file.close();
}



/*************************************************************************************************/
/**
	SourceFile::Process()

	Process the associated source file
*/
/*************************************************************************************************/
void SourceFile::Process()
{
	// Initialise for stack

	m_forStackPtr = 0;

	// Initialise if

	m_ifStackPtr = 0;

	// Iterate through the file line-by-line

	string lineFromFile;

	while ( getline( m_file, lineFromFile ) )
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
			e.SetFilename( m_pFilename );
			e.SetLineNumber( m_lineNumber );
			throw;
		}

		m_lineNumber++;
		m_filePointer = m_file.tellg();
	}

	// Check whether we aborted prematurely

	if ( !m_file.eof() )
	{
		throw AsmException_FileError_ReadSourceFile( m_pFilename );
	}

	// Check that we have no FOR / braces mismatch

	if ( m_forStackPtr > 0 )
	{
		For& mismatchedFor = m_forStack[ m_forStackPtr - 1 ];

		if ( mismatchedFor.m_step == 0.0 )
		{
			AsmException_SyntaxError_MismatchedBraces e( mismatchedFor.m_line, mismatchedFor.m_column );
			e.SetFilename( m_pFilename );
			e.SetLineNumber( mismatchedFor.m_lineNumber );
			throw e;
		}
		else
		{
			AsmException_SyntaxError_ForWithoutNext e( mismatchedFor.m_line, mismatchedFor.m_column );
			e.SetFilename( m_pFilename );
			e.SetLineNumber( mismatchedFor.m_lineNumber );
			throw e;
		}
	}

	// Check that we have no IF mismatch

	if ( m_ifStackPtr > 0 )
	{
		If& mismatchedIf = m_ifStack[ m_ifStackPtr - 1 ];

		AsmException_SyntaxError_IfWithoutEndif e( mismatchedIf.m_line, mismatchedIf.m_column );
		e.SetFilename( m_pFilename );
		e.SetLineNumber( mismatchedIf.m_lineNumber );
		throw e;
	}

	// Display ok message

	if ( GlobalData::Instance().IsFirstPass() )
	{
		cerr << "Processed file '" << m_pFilename << "' ok" << endl << endl;
	}
}



/*************************************************************************************************/
/**
	SourceFile::AddFor()
*/
/*************************************************************************************************/
void SourceFile::AddFor( string varName,
						 double start,
						 double end,
						 double step,
						 int filePtr,
						 string line,
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
	SourceFile::OpenBrace()

	Braces for scoping variables are just FORs in disguise...
*/
/*************************************************************************************************/
void SourceFile::OpenBrace( string line, int column )
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
	SourceFile::UpdateFor()
*/
/*************************************************************************************************/
void SourceFile::UpdateFor( string line, int column )
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
	SourceFile::CloseBrace()

	Braces for scoping variables are just FORs in disguise...
*/
/*************************************************************************************************/
void SourceFile::CloseBrace( string line, int column )
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
	SourceFile::GetSymbolNameSuffix()
*/
/*************************************************************************************************/
string SourceFile::GetSymbolNameSuffix( int level ) const
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
	SourceFile::IsIfConditionTrue()
*/
/*************************************************************************************************/
bool SourceFile::IsIfConditionTrue() const
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
	SourceFile::AddIfLevel()
*/
/*************************************************************************************************/
void SourceFile::AddIfLevel( string line, int column )
{
	if ( m_ifStackPtr == MAX_IF_LEVELS )
	{
		throw AsmException_SyntaxError_TooManyIFs( line, column );
	}

	m_ifStack[ m_ifStackPtr ].m_condition	= true;
	m_ifStack[ m_ifStackPtr ].m_line		= line;
	m_ifStack[ m_ifStackPtr ].m_column		= column;
	m_ifStack[ m_ifStackPtr ].m_lineNumber	= m_lineNumber;
	m_ifStackPtr++;
}



/*************************************************************************************************/
/**
	SourceFile::SetCurrentIfCondition()
*/
/*************************************************************************************************/
void SourceFile::SetCurrentIfCondition( bool b )
{
	assert( m_ifStackPtr > 0 );
	m_ifStack[ m_ifStackPtr - 1 ].m_condition = b;
}



/*************************************************************************************************/
/**
	SourceFile::ToggleCurrentIfCondition()
*/
/*************************************************************************************************/
void SourceFile::ToggleCurrentIfCondition( string line, int column )
{
	if ( m_ifStackPtr == 0 )
	{
		throw AsmException_SyntaxError_ElseWithoutIf( line, column );
	}

	m_ifStack[ m_ifStackPtr - 1 ].m_condition = !m_ifStack[ m_ifStackPtr - 1 ].m_condition;
}



/*************************************************************************************************/
/**
	SourceFile::RemoveIfLevel()
*/
/*************************************************************************************************/
void SourceFile::RemoveIfLevel( string line, int column )
{
	if ( m_ifStackPtr == 0 )
	{
		throw AsmException_SyntaxError_EndifWithoutIf( line, column );
	}

	m_ifStackPtr--;
}
