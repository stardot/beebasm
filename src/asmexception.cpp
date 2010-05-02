/*************************************************************************************************/
/**
	asmexception.cpp

	Exception handling for the app
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
	cerr << "Error: " << m_pFilename << ": " << Message() << endl;
}



/*************************************************************************************************/
/**
	AsmException_SyntaxError::Print()

	Outputs to stderr an error message regarding a syntax error
*/
/*************************************************************************************************/
void AsmException_SyntaxError::Print() const
{
	assert( m_pFilename != NULL );
	assert( m_lineNumber != 0 );

	cerr << m_pFilename << ":" << m_lineNumber << ": error: ";
	cerr << Message() << endl << endl;
	cerr << m_line << endl;
	cerr << string( m_column, ' ' ) << "^" << endl;
}

