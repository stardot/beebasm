/*************************************************************************************************/
/**
	commands.cpp

	Contains all the LineParser methods for parsing and handling assembler commands


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
#include <string>
#include <cstring>

#include "lineparser.h"
#include "globaldata.h"
#include "objectcode.h"
#include "symboltable.h"
#include "sourcefile.h"
#include "asmexception.h"
#include "discimage.h"
#include "BASIC.h"


using namespace std;

const LineParser::Token	LineParser::m_gaTokenTable[] =
{
	{ ".",			&LineParser::HandleDefineLabel,			0 }, // Why is gcc forcing me to
	{ "\\",			&LineParser::HandleDefineComment,		0 }, // put all these 0s in?
	{ ";",			&LineParser::HandleDefineComment,		0 },
	{ ":",			&LineParser::HandleStatementSeparator,	0 },
	{ "PRINT",		&LineParser::HandlePrint,				0 },
	{ "CPU",		&LineParser::HandleCpu,					0 },
	{ "ORG",		&LineParser::HandleOrg,					0 },
	{ "INCLUDE",	&LineParser::HandleInclude,				0 },
	{ "EQUB",		&LineParser::HandleEqub,				0 },
	{ "EQUD",		&LineParser::HandleEqud,				0 },
	{ "EQUS",		&LineParser::HandleEqub,				0 },
	{ "EQUW",		&LineParser::HandleEquw,				0 },
	{ "ASSERT",		&LineParser::HandleAssert,				0 },
	{ "SAVE",		&LineParser::HandleSave,				0 },
	{ "FOR",		&LineParser::HandleFor,					0 },
	{ "NEXT",		&LineParser::HandleNext,				0 },
	{ "IF",			&LineParser::HandleIf,					&SourceFile::AddIfLevel },
	{ "ELIF",		&LineParser::HandleIf,					&SourceFile::StartElif },
	{ "ELSE",		&LineParser::HandleDirective,			&SourceFile::StartElse },
	{ "ENDIF",		&LineParser::HandleDirective,			&SourceFile::RemoveIfLevel },
	{ "ALIGN",		&LineParser::HandleAlign,				0 },
	{ "SKIPTO",		&LineParser::HandleSkipTo,				0 },
	{ "SKIP",		&LineParser::HandleSkip,				0 },
	{ "GUARD",		&LineParser::HandleGuard,				0 },
	{ "CLEAR",		&LineParser::HandleClear,				0 },
	{ "INCBIN",		&LineParser::HandleIncBin,				0 },
	{ "{",			&LineParser::HandleOpenBrace,			0 },
	{ "}",			&LineParser::HandleCloseBrace,			0 },
	{ "MAPCHAR",	&LineParser::HandleMapChar,				0 },
	{ "PUTFILE",	&LineParser::HandlePutFile,				0 },
	{ "PUTBASIC",	&LineParser::HandlePutBasic,			0 },
	{ "MACRO",		&LineParser::HandleMacro,				&SourceFile::StartMacro },
	{ "ENDMACRO",	&LineParser::HandleEndMacro,			&SourceFile::EndMacro },
	{ "ERROR",		&LineParser::HandleError,				0 },
	{ "COPYBLOCK",	&LineParser::HandleCopyBlock,			0 }
};




/*************************************************************************************************/
/**
	LineParser::GetTokenAndAdvanceColumn()

	Searches for a token match in the current line, starting at the current column,
	and moves the column pointer past the token

	@param		line			The string to parse
	@param		column			The column to start from

	@return		The token number, or -1 for "not found"
				column is modified to index the character after the token
*/
/*************************************************************************************************/
int LineParser::GetTokenAndAdvanceColumn()
{
	for ( int i = 0; i < static_cast<int>( sizeof m_gaTokenTable / sizeof( Token ) ); i++ )
	{
		const char*	token	= m_gaTokenTable[ i ].m_pName;
		size_t		len		= strlen( token );

		// see if token matches

		bool bMatch = true;
		for ( unsigned int j = 0; j < len; j++ )
		{
			if ( token[ j ] != toupper( m_line[ m_column + j ] ) )
			{
				bMatch = false;
				break;
			}
		}

		if ( bMatch )
		{
			m_column += len;
			return i;
		}
	}

	return -1;
}



/*************************************************************************************************/
/**
	LineParser::HandleDefineLabel()
*/
/*************************************************************************************************/
void LineParser::HandleDefineLabel()
{
	if ( m_column < m_line.length() && ( isalpha( m_line[ m_column ] ) || m_line[ m_column ] == '_' ) )
	{
		// Symbol starts with a valid character

		int oldColumn = m_column;

		// Get the symbol name

		string symbolName = GetSymbolName();

		// ...and mangle it according to whether we are in a FOR loop

		string fullSymbolName = symbolName + m_sourceCode->GetSymbolNameSuffix();

		if ( GlobalData::Instance().IsFirstPass() )
		{
			// only add the symbol on the first pass

			if ( SymbolTable::Instance().IsSymbolDefined( fullSymbolName ) )
			{
				throw AsmException_SyntaxError_LabelAlreadyDefined( m_line, oldColumn );
			}
			else
			{
				SymbolTable::Instance().AddSymbol( fullSymbolName, ObjectCode::Instance().GetPC(), true );
			}
		}
		else
		{
			// on the second pass, check that the label would be assigned the same value

			if ( SymbolTable::Instance().GetSymbol( fullSymbolName ) != ObjectCode::Instance().GetPC() )
			{
				throw AsmException_SyntaxError_SecondPassProblem( m_line, oldColumn );
			}
		}

		if ( GlobalData::Instance().ShouldOutputAsm() )
		{
			cout << "." << symbolName << endl;
		}
	}
	else
	{
		throw AsmException_SyntaxError_InvalidSymbolName( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleDefineComment()
*/
/*************************************************************************************************/
void LineParser::HandleDefineComment()
{
	// skip rest of line
	m_column = m_line.length();
}



/*************************************************************************************************/
/**
	LineParser::HandleStatementSeparator()
*/
/*************************************************************************************************/
void LineParser::HandleStatementSeparator()
{
	// do nothing!
}



/*************************************************************************************************/
/**
	LineParser::HandleDirective()
*/
/*************************************************************************************************/
void LineParser::HandleDirective()
{
	// directive that doesn't need extra handling; just check rest of line is empty
	if ( AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleOrg()
*/
/*************************************************************************************************/
void LineParser::HandleOrg()
{
	int newPC = EvaluateExpressionAsInt();
	if ( newPC < 0 || newPC > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	ObjectCode::Instance().SetPC( newPC );
	SymbolTable::Instance().ChangeSymbol( "P%", newPC );

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleCpu()
*/
/*************************************************************************************************/
void LineParser::HandleCpu()
{
	int newCpu = EvaluateExpressionAsInt();
	if ( newCpu < 0 || newCpu > 1 )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	ObjectCode::Instance().SetCPU( newCpu );

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleGuard()
*/
/*************************************************************************************************/
void LineParser::HandleGuard()
{
	int val = EvaluateExpressionAsInt();
	if ( val < 0 || val > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	ObjectCode::Instance().SetGuard( val );

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleClear()
*/
/*************************************************************************************************/
void LineParser::HandleClear()
{
	int start = EvaluateExpressionAsInt();
	if ( start < 0 || start > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
	{
		// did not find a comma
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	int end = EvaluateExpressionAsInt();
	if ( end < 0 || end > 0x10000 )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	ObjectCode::Instance().Clear( start, end );

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleMapChar()
*/
/*************************************************************************************************/
void LineParser::HandleMapChar()
{
	// get parameters - either 2 or 3

	int param3 = -1;
	int param1 = EvaluateExpressionAsInt();

	if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
	{
		// did not find a comma
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	int param2 = EvaluateExpressionAsInt();

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		m_column++;

		param3 = EvaluateExpressionAsInt();
	}

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}

	// range checks

	if ( param1 < 32 || param1 > 126 )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	if ( param3 == -1 )
	{
		// two parameters

		if ( param2 < 0 || param2 > 255 )
		{
			throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
		}

		// do single character remapping
		ObjectCode::Instance().SetMapping( param1, param2 );
	}
	else
	{
		// three parameters

		if ( param2 < 32 || param2 > 126 || param2 < param1 )
		{
			throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
		}

		if ( param3 < 0 || param3 > 255 )
		{
			throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
		}

		// remap a block
		for ( int i = param1; i <= param2; i++ )
		{
			ObjectCode::Instance().SetMapping( i, param3 + i - param1 );
		}
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleAlign()
*/
/*************************************************************************************************/
void LineParser::HandleAlign()
{
	int oldColumn = m_column;

	int val = EvaluateExpressionAsInt();
	if ( val < 1 || ( val & ( val - 1 ) ) != 0 )
	{
		throw AsmException_SyntaxError_BadAlignment( m_line, oldColumn );
	}

	while ( ( ObjectCode::Instance().GetPC() & ( val - 1 ) ) != 0 )
	{
		try
		{
			ObjectCode::Instance().PutByte( 0 );
		}
		catch ( AsmException_AssembleError& e )
		{
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw;
		}
	}

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleSkip()
*/
/*************************************************************************************************/
void LineParser::HandleSkip()
{
	int oldColumn = m_column;

	int val = EvaluateExpressionAsInt();
	if ( val < 0 )
	{
		throw AsmException_SyntaxError_ImmNegative( m_line, oldColumn );
	}

	if ( GlobalData::Instance().ShouldOutputAsm() )
	{
		cout << uppercase << hex << setfill( '0' ) << "     ";
		cout << setw(4) << ObjectCode::Instance().GetPC() << endl;
	}

	for ( int i = 0; i < val; i++ )
	{
		try
		{
			ObjectCode::Instance().PutByte( 0 );
		}
		catch ( AsmException_AssembleError& e )
		{
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw;
		}
	}

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleSkipTo()
*/
/*************************************************************************************************/
void LineParser::HandleSkipTo()
{
	int oldColumn = m_column;

	int addr = EvaluateExpressionAsInt();
	if ( addr < 0 || addr > 0x10000 )
	{
		throw AsmException_SyntaxError_BadAddress( m_line, oldColumn );
	}

	if ( ObjectCode::Instance().GetPC() > addr )
	{
		throw AsmException_SyntaxError_BackwardsSkip( m_line, oldColumn );
	}

	while ( ObjectCode::Instance().GetPC() < addr )
	{
		try
		{
			ObjectCode::Instance().PutByte( 0 );
		}
		catch ( AsmException_AssembleError& e )
		{
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw;
		}
	}

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleInclude()
*/
/*************************************************************************************************/
void LineParser::HandleInclude()
{
	if ( m_sourceCode->GetForLevel() > 0 )
	{
		// disallow an include within a FOR loop
		throw AsmException_SyntaxError_CantInclude( m_line, m_column );
	}

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_column >= m_line.length() || m_line[ m_column ] != '\"' )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	// string
	size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

	if ( endQuotePos == string::npos )
	{
		throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
	}
	else
	{
		string filename( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );

		if ( GlobalData::Instance().ShouldOutputAsm() )
		{
			cerr << "Including file " << filename << endl;
		}

		SourceFile input( filename.c_str() );
		input.Process();
	}

	m_column = endQuotePos + 1;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleIncBin()
*/
/*************************************************************************************************/
void LineParser::HandleIncBin()
{
	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_column >= m_line.length() || m_line[ m_column ] != '\"' )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	// string
	size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

	if ( endQuotePos == string::npos )
	{
		throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
	}
	else
	{
		string filename( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );

		try
		{
			ObjectCode::Instance().IncBin( filename.c_str() );
		}
		catch ( AsmException_AssembleError& e )
		{
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw;
		}
	}

	m_column = endQuotePos + 1;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleEqub()
*/
/*************************************************************************************************/
void LineParser::HandleEqub()
{
	do
	{
		if ( !AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

		// handle string

		if ( m_column < m_line.length() && m_line[ m_column ] == '\"' )
		{
			size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

			if ( endQuotePos == string::npos )
			{
				throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
			}
			else
			{
				string equs( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );

				if ( GlobalData::Instance().ShouldOutputAsm() )
				{
					cout << uppercase << hex << setfill( '0' ) << "     ";
					cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
				}

				for ( size_t i = 0; i < equs.length(); i++ )
				{
					int mappedchar = ObjectCode::Instance().GetMapping( equs[ i ] );

					if ( GlobalData::Instance().ShouldOutputAsm() )
					{
						if ( i < 3 )
						{
							cout << setw(2) << mappedchar << " ";
						}
						else if ( i == 3 )
						{
							cout << "...";
						}
					}

					try
					{
						// remap character from string as per character mapping table
						ObjectCode::Instance().PutByte( mappedchar );
					}
					catch ( AsmException_AssembleError& e )
					{
						e.SetString( m_line );
						e.SetColumn( m_column );
						throw;
					}
				}

				if ( GlobalData::Instance().ShouldOutputAsm() )
				{
					cout << endl << nouppercase << dec << setfill( ' ' );
				}
			}

			m_column = endQuotePos + 1;
		}
		else
		{
			// handle byte

			int value;

			try
			{
				value = EvaluateExpressionAsInt();
			}
			catch ( AsmException_SyntaxError_SymbolNotDefined& )
			{
				if ( GlobalData::Instance().IsFirstPass() )
				{
					value = 0;
				}
				else
				{
					throw;
				}
			}

			if ( value > 0xFF )
			{
				throw AsmException_SyntaxError_NumberTooBig( m_line, m_column );
			}

			if ( GlobalData::Instance().ShouldOutputAsm() )
			{
				cout << uppercase << hex << setfill( '0' ) << "     ";
				cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
				cout << setw(2) << ( value & 0xFF );
				cout << endl << nouppercase << dec << setfill( ' ' );
			}

			try
			{
				ObjectCode::Instance().PutByte( value & 0xFF );
			}
			catch ( AsmException_AssembleError& e )
			{
				e.SetString( m_line );
				e.SetColumn( m_column );
				throw;
			}
		}

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			break;
		}

		if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;

	} while ( true );
}



/*************************************************************************************************/
/**
	LineParser::HandleEquw()
*/
/*************************************************************************************************/
void LineParser::HandleEquw()
{
	do
	{
		int value;

		try
		{
			value = EvaluateExpressionAsInt();
		}
		catch ( AsmException_SyntaxError_SymbolNotDefined& )
		{
			if ( GlobalData::Instance().IsFirstPass() )
			{
				value = 0;
			}
			else
			{
				throw;
			}
		}

		if ( value > 0xFFFF )
		{
			throw AsmException_SyntaxError_NumberTooBig( m_line, m_column );
		}

		if ( GlobalData::Instance().ShouldOutputAsm() )
		{
			cout << uppercase << hex << setfill( '0' ) << "     ";
			cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
			cout << setw(2) << ( value & 0xFF ) << " ";
			cout << setw(2) << ( ( value & 0xFF00 ) >> 8 );
			cout << endl << nouppercase << dec << setfill( ' ' );
		}

		try
		{
			ObjectCode::Instance().PutByte( value & 0xFF );
			ObjectCode::Instance().PutByte( ( value & 0xFF00 ) >> 8 );
		}
		catch ( AsmException_AssembleError& e )
		{
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw;
		}

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			break;
		}

		if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

	} while ( true );
}



/*************************************************************************************************/
/**
	LineParser::HandleEqud()
*/
/*************************************************************************************************/
void LineParser::HandleEqud()
{
	do
	{
		unsigned int value;

		try
		{
			value = EvaluateExpressionAsUnsignedInt();
		}
		catch ( AsmException_SyntaxError_SymbolNotDefined& )
		{
			if ( GlobalData::Instance().IsFirstPass() )
			{
				value = 0;
			}
			else
			{
				throw;
			}
		}

		if ( GlobalData::Instance().ShouldOutputAsm() )
		{
			cout << uppercase << hex << setfill( '0' ) << "     ";
			cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
			cout << setw(2) << ( value & 0xFF ) << " ";
			cout << setw(2) << ( ( value & 0xFF00 ) >> 8 ) << " ";
			cout << setw(2) << ( ( value & 0xFF0000 ) >> 16 ) << " ";
			cout << setw(2) << ( ( value & 0xFF000000 ) >> 24 );
			cout << endl << nouppercase << dec << setfill( ' ' );
		}

		try
		{
			ObjectCode::Instance().PutByte( value & 0xFF );
			ObjectCode::Instance().PutByte( ( value & 0xFF00 ) >> 8 );
			ObjectCode::Instance().PutByte( ( value & 0xFF0000 ) >> 16 );
			ObjectCode::Instance().PutByte( ( value & 0xFF000000 ) >> 24 );
		}
		catch ( AsmException_AssembleError& e )
		{
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw;
		}

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			break;
		}

		if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

	} while ( true );
}



/*************************************************************************************************/
/**
	LineParser::HandleAssert()
*/
/*************************************************************************************************/
void LineParser::HandleAssert()
{
	do
	{
		unsigned int value;

		try
		{
			// Take a copy of the column before evaluating the expression so
			// we can point correctly at the failed expression when throwing.
			size_t column = m_column;
			value = EvaluateExpressionAsUnsignedInt();
			// We never throw for value being false on the first pass, simply
			// to ensure that if two assertions both fail, the one which 
			// appears earliest in the source will be reported.
			if ( !GlobalData::Instance().IsFirstPass() && !value )
			{
				while ( ( column < m_line.length() ) && isspace( static_cast< unsigned char >( m_line[ column ] ) ) )
				{
					column++;
				}

				throw AsmException_SyntaxError_AssertionFailed( m_line, column );
			}
		}
		catch ( AsmException_SyntaxError_SymbolNotDefined& )
		{
			if ( !GlobalData::Instance().IsFirstPass() )
			{
				throw;
			}
		}

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			break;
		}

		if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

	} while ( true );
}



/*************************************************************************************************/
/**
	LineParser::HandleSave()
*/
/*************************************************************************************************/
void LineParser::HandleSave()
{
	int start = 0;
	int end = 0;
	int exec = 0;
	int reload = 0;

	int oldColumn = m_column;

	// syntax is SAVE "filename", start, end [, exec [, reload] ]

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	string saveFile;

	if ( m_line[ m_column ] == '\"' )
	{
		// get filename

		size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

		if ( endQuotePos == string::npos )
		{
			// did not find the end of the string
			throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
		}

		saveFile = m_line.substr( m_column + 1, endQuotePos - m_column - 1 );

		m_column = endQuotePos + 1;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			// found nothing
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

		if ( m_line[ m_column ] != ',' )
		{
			// did not find a comma
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;
	}

	// get start address

	start = EvaluateExpressionAsInt();
	exec = start;
	reload = start;

	if ( start < 0 || start > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	// get end address

	if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
	{
		// did not find a comma
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	end = EvaluateExpressionAsInt();

	if ( end < 0 || end > 0x10000 )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	// get optional exec address
	// we allow this to be a forward define as it needn't be within the block we actually save

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		m_column++;

		try
		{
			exec = EvaluateExpressionAsInt();
		}
		catch ( AsmException_SyntaxError_SymbolNotDefined& )
		{
			if ( GlobalData::Instance().IsSecondPass() )
			{
				throw;
			}
		}

		if ( exec < 0 || exec > 0xFFFF )
		{
			throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
		}

		// get optional reload address

		if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
		{
			m_column++;

			reload = EvaluateExpressionAsInt();

			if ( reload < 0 || reload > 0xFFFF )
			{
				throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
			}
		}
	}

	// expect no more

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}

	if ( saveFile == "" )
	{
		if ( GlobalData::Instance().GetOutputFile() != NULL )
		{
			saveFile = GlobalData::Instance().GetOutputFile();

			if ( GlobalData::Instance().IsSecondPass() )
			{
				if ( GlobalData::Instance().GetNumAnonSaves() > 0 )
				{
					throw AsmException_SyntaxError_OnlyOneAnonSave( m_line, oldColumn );
				}
				else
				{
					GlobalData::Instance().IncNumAnonSaves();
				}
			}
		}
		else
		{
			throw AsmException_SyntaxError_NoAnonSave( m_line, oldColumn );
		}
	}

	if ( GlobalData::Instance().ShouldOutputAsm() )
	{
		cout << "Saving file '" << saveFile << "'" << endl;
	}

	// OK - do it

	if ( GlobalData::Instance().IsSecondPass() )
	{
		if ( GlobalData::Instance().UsesDiscImage() )
		{
			// disc image version of the save
			GlobalData::Instance().GetDiscImage()->AddFile( saveFile.c_str(),
															ObjectCode::Instance().GetAddr( start ),
															reload,
															exec,
															end - start );
		}
		else
		{
			// regular save
			ofstream objFile;

			objFile.open( saveFile.c_str(), ios_base::out | ios_base::binary | ios_base::trunc );

			if ( !objFile )
			{
				throw AsmException_FileError_OpenObj( saveFile.c_str() );
			}

			if ( !objFile.write( reinterpret_cast< const char* >( ObjectCode::Instance().GetAddr( start ) ), end - start ) )
			{
				throw AsmException_FileError_WriteObj( saveFile.c_str() );
			}

			objFile.close();
		}

		GlobalData::Instance().SetSaved();
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleFor()
*/
/*************************************************************************************************/
void LineParser::HandleFor()
{
	// syntax is FOR variable, exp, exp [, exp]

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	// first look for the variable name

	if ( !isalpha( m_line[ m_column ] ) && m_line[ m_column ] != '_' )
	{
		throw AsmException_SyntaxError_InvalidSymbolName( m_line, m_column );
	}

	// Symbol starts with a valid character

	int oldColumn = m_column;
	string symbolName = GetSymbolName() + m_sourceCode->GetSymbolNameSuffix();

	// Check variable has not yet been defined

	if ( SymbolTable::Instance().IsSymbolDefined( symbolName ) )
	{
		throw AsmException_SyntaxError_LabelAlreadyDefined( m_line, oldColumn );
	}

	// look for first comma

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != ',' )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
	m_column++;

	// look for start value

	double start = EvaluateExpression();

	// look for comma

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != ',' )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	// look for end value

	double end = EvaluateExpression();

	double step = 1.0;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		// look for step variable

		if ( m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;

		step = EvaluateExpression();

		if ( step == 0.0 )
		{
			throw AsmException_SyntaxError_BadStep( m_line, m_column );
		}

		// check this is now the end

		if ( AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

	}

	m_sourceCode->AddFor( symbolName,
						  start, end, step,
						  m_sourceCode->GetLineStartPointer() + m_column,
						  m_line,
						  oldColumn );
}



/*************************************************************************************************/
/**
	LineParser::HandleOpenBrace()
*/
/*************************************************************************************************/
void LineParser::HandleOpenBrace()
{
	m_sourceCode->OpenBrace( m_line, m_column - 1 );
}



/*************************************************************************************************/
/**
	LineParser::HandleNext()
*/
/*************************************************************************************************/
void LineParser::HandleNext()
{
	int oldColumn = m_column;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_sourceCode->UpdateFor( m_line, oldColumn );
}



/*************************************************************************************************/
/**
	LineParser::HandleCloseBrace()

	Braces for scoping variables are just FORs in disguise...
*/
/*************************************************************************************************/
void LineParser::HandleCloseBrace()
{
	m_sourceCode->CloseBrace( m_line, m_column - 1 );
}



/*************************************************************************************************/
/**
	LineParser::HandleIf()
*/
/*************************************************************************************************/
void LineParser::HandleIf()
{
	// Handles both IF and ELIF
	bool condition = (EvaluateExpressionAsInt() != 0);
	m_sourceCode->SetCurrentIfCondition( condition );

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandlePrint()

	Handle PRINT token.

	Valid syntax:
	  PRINT
	  PRINT "string"
	  PRINT 2+7
	  PRINT someLabel
	  PRINT "someLabel =", someLabel, "someLabel in hex =", ~someLabel
*/
/*************************************************************************************************/
void LineParser::HandlePrint()
{
	bool bDemandComma = false;

	while ( AdvanceAndCheckEndOfStatement() )
	{
		if ( m_line[ m_column ] == ',' )
		{
			// print separator - skip
			bDemandComma = false;
			m_column++;
		}
		else if ( bDemandComma )
		{
			throw AsmException_SyntaxError_MissingComma( m_line, m_column );
		}
		else if ( m_line[ m_column ] == '\"' )
		{
			// string
			size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

			if ( endQuotePos == string::npos )
			{
				throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
			}
			else
			{
				if ( GlobalData::Instance().IsSecondPass() )
				{
					cout << m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) << " ";
				}
			}

			m_column = endQuotePos + 1;
			bDemandComma = true;
		}
		else if ( m_line[ m_column ] == '~' )
		{
			// print in hex
			m_column++;

			int value;

			try
			{
				value = EvaluateExpressionAsInt();
			}
			catch ( AsmException_SyntaxError_SymbolNotDefined& )
			{
				if ( GlobalData::Instance().IsFirstPass() )
				{
					value = 0;
				}
				else
				{
					throw;
				}
			}

			if ( GlobalData::Instance().IsSecondPass() )
			{
				cout << hex << uppercase << "&" << value << dec << nouppercase << " ";
			}
		}
		else
		{
			// print in dec

			double value;

			try
			{
				value = EvaluateExpression();
			}
			catch ( AsmException_SyntaxError_SymbolNotDefined& )
			{
				if ( GlobalData::Instance().IsFirstPass() )
				{
					value = 0.0;
				}
				else
				{
					throw;
				}
			}

			if ( GlobalData::Instance().IsSecondPass() )
			{
				cout << value << " ";
			}
		}
	}

	if ( GlobalData::Instance().IsSecondPass() )
	{
		cout << endl;
	}
}



/*************************************************************************************************/
/**
	LineParser::HandlePutFile()
*/
/*************************************************************************************************/
void LineParser::HandlePutFile()
{
	// Syntax:
	// PUTFILE <host filename>, [<beeb filename>,] <start addr> [,<exec addr>]

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != '\"' )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	// get first filename
	size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

	if ( endQuotePos == string::npos )
	{
		throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
	}

	string hostFilename( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );
	string beebFilename = hostFilename;
	int start = 0;
	int exec = 0;

	m_column = endQuotePos + 1;

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != ',' )
	{
		throw AsmException_SyntaxError_MissingComma( m_line, m_column );
	}

	m_column++;

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] == '\"' )
	{
		// string
		endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

		if ( endQuotePos == string::npos )
		{
			throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
		}

		// get the second filename parameter

		beebFilename = m_line.substr( m_column + 1, endQuotePos - m_column - 1 );

		m_column = endQuotePos + 1;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			// found nothing
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

		if ( m_line[ m_column ] != ',' )
		{
			// did not find a comma
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		m_column++;
	}

	// Get start address

	try
	{
		start = EvaluateExpressionAsInt();
	}
	catch ( AsmException_SyntaxError_SymbolNotDefined& )
	{
		if ( GlobalData::Instance().IsSecondPass() )
		{
			throw;
		}
	}

	exec = start;

	if ( start < 0 || start > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		m_column++;

		try
		{
			exec = EvaluateExpressionAsInt();
		}
		catch ( AsmException_SyntaxError_SymbolNotDefined& )
		{
			if ( GlobalData::Instance().IsSecondPass() )
			{
				throw;
			}
		}

		if ( exec < 0 || exec > 0xFFFF )
		{
			throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
		}
	}

	// check this is now the end

	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	if ( GlobalData::Instance().IsSecondPass() )
	{
		ifstream inputFile;
		inputFile.open( hostFilename.c_str(), ios_base::in | ios_base::binary );

		if ( !inputFile )
		{
			AsmException_AssembleError_FileOpen e;
			e.SetString( m_line );
			e.SetColumn( m_column );
			throw e;
		}

		inputFile.seekg( 0, ios_base::end );
		size_t fileSize = static_cast< size_t >( inputFile.tellg() );
		inputFile.seekg( 0, ios_base::beg );

		char* buffer = new char[ fileSize ];
		inputFile.read( buffer, fileSize );
		inputFile.close();

		if ( GlobalData::Instance().UsesDiscImage() )
		{
			// disc image version of the save
			GlobalData::Instance().GetDiscImage()->AddFile( beebFilename.c_str(),
															reinterpret_cast< unsigned char* >( buffer ),
															start,
															exec,
															fileSize );
		}

		delete [] buffer;
	}
}


/*************************************************************************************************/
/**
	LineParser::HandlePutBasic()
*/
/*************************************************************************************************/
void LineParser::HandlePutBasic()
{
	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != '\"' )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	// get first filename
	size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

	if ( endQuotePos == string::npos )
	{
		throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
	}

	string hostFilename( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );
	string beebFilename = hostFilename;

	m_column = endQuotePos + 1;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		// see if there's a second parameter

		if ( m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_MissingComma( m_line, m_column );
		}

		m_column++;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

		if ( m_line[ m_column ] != '\"' )
		{
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}

		// string
		endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

		if ( endQuotePos == string::npos )
		{
			throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
		}

		// get the second parameter

		beebFilename = m_line.substr( m_column + 1, endQuotePos - m_column - 1 );

		m_column = endQuotePos + 1;
	}

	// check this is now the end

	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	if ( GlobalData::Instance().IsSecondPass() &&
		 GlobalData::Instance().UsesDiscImage() )
	{
		Uint8* buffer = new Uint8[ 0x10000 ];
		int fileSize;
		bool bSuccess = ImportBASIC( hostFilename.c_str(), buffer, &fileSize );

		if (!bSuccess)
		{
			if (GetBASICErrorNum() == 2)
			{
				AsmException_AssembleError_FileOpen e;
				e.SetString( m_line );
				e.SetColumn( m_column );
				throw e;
			}
			else
			{
				std::string message = hostFilename + ": " + GetBASICError();
				throw AsmException_UserError( m_line, m_column, message );
			}
		}

		// disc image version of the save
		GlobalData::Instance().GetDiscImage()->AddFile( beebFilename.c_str(),
														reinterpret_cast< unsigned char* >( buffer ),
														0xFFFF1900,
														0xFFFF8023,
														fileSize );

		delete [] buffer;
	}

}


/*************************************************************************************************/
/**
	LineParser::HandleMacro()
*/
/*************************************************************************************************/
void LineParser::HandleMacro()
{
	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	string macroName;

	if ( isalpha( m_line[ m_column ] ) || m_line[ m_column ] == '_' )
	{
		macroName = GetSymbolName();

		if ( GlobalData::Instance().IsFirstPass() )
		{
			if ( MacroTable::Instance().Exists( macroName ) )
			{
				throw AsmException_SyntaxError_DuplicateMacroName( m_line, m_column );
			}

			m_sourceCode->GetCurrentMacro()->SetName( macroName );
		}
	}
	else
	{
		throw AsmException_SyntaxError_InvalidMacroName( m_line, m_column );
	}

	bool bExpectComma = false;
	bool bHasParameters = false;

	while ( AdvanceAndCheckEndOfStatement() )
	{
		if ( bExpectComma )
		{
			if ( m_line[ m_column ] == ',' )
			{
				m_column++;
				bExpectComma = false;
			}
			else
			{
				throw AsmException_SyntaxError_MissingComma( m_line, m_column );
			}
		}
		else if ( isalpha( m_line[ m_column ] ) || m_line[ m_column ] == '_' )
		{
			string param = GetSymbolName();

			if ( GlobalData::Instance().IsFirstPass() )
			{
				m_sourceCode->GetCurrentMacro()->AddParameter( param );
			}
			bExpectComma = true;
			bHasParameters = true;
		}
		else
		{
			throw AsmException_SyntaxError_InvalidSymbolName( m_line, m_column );
		}
	}

	if ( bHasParameters && !bExpectComma )
	{
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column - 1 );
	}

	// If there is nothing else on the line following the MACRO command, put a newline at the
	// beginning of the macro definition, so any errors are reported on the correct line

	if ( m_column == m_line.length() &&
		 GlobalData::Instance().IsFirstPass() )
	{
		m_sourceCode->GetCurrentMacro()->AddLine("\n");
	}

	// Set the IF condition to false - this is a cheaty way of ensuring that the macro body
	// is not assembled as it is parsed

	m_sourceCode->SetCurrentIfCondition(false);
}


/*************************************************************************************************/
/**
	LineParser::HandleEndMacro()
*/
/*************************************************************************************************/
void LineParser::HandleEndMacro()
{
	if ( AdvanceAndCheckEndOfStatement() )
	{
		// found something
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
}


/*************************************************************************************************/
/**
	LineParser::HandleError()
*/
/*************************************************************************************************/
void LineParser::HandleError()
{
	int oldColumn = m_column;

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_column >= m_line.length() || m_line[ m_column ] != '\"' )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	// string
	size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

	if ( endQuotePos == string::npos )
	{
		throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
	}
	else
	{
		string errorMsg( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );

		// throw error

		throw AsmException_UserError( m_line, oldColumn, errorMsg );
	}

	m_column = endQuotePos + 1;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
}


/*************************************************************************************************/
/**
	LineParser::HandleCopyBlock()
*/
/*************************************************************************************************/
void LineParser::HandleCopyBlock()
{
	int start = EvaluateExpressionAsInt();
	if ( start < 0 || start > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
	{
		// did not find a comma
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	int end = EvaluateExpressionAsInt();
	if ( end < 0 || end > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
	{
		// did not find a comma
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	int dest = EvaluateExpressionAsInt();
	if ( dest < 0 || dest > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	if ( GlobalData::Instance().IsSecondPass() )
	{
		ObjectCode::Instance().CopyBlock( start, end, dest );
	}

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}

