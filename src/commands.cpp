/*************************************************************************************************/
/**
	commands.cpp

	Contains all the LineParser methods for parsing and handling assembler commands
*/
/*************************************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>

#include "lineparser.h"
#include "globaldata.h"
#include "objectcode.h"
#include "symboltable.h"
#include "sourcefile.h"
#include "asmexception.h"
#include "discimage.h"


using namespace std;


LineParser::Token	LineParser::m_gaTokenTable[] =
{
	{ ".",			&LineParser::HandleDefineLabel },
	{ "\\",			&LineParser::HandleDefineComment },
	{ ";",			&LineParser::HandleDefineComment },
	{ ":",			&LineParser::HandleStatementSeparator },
	{ "PRINT",		&LineParser::HandlePrint },
	{ "ORG",		&LineParser::HandleOrg },
	{ "INCLUDE",	&LineParser::HandleInclude },
	{ "EQUB",		&LineParser::HandleEqub },
	{ "EQUS",		&LineParser::HandleEqub },
	{ "EQUW",		&LineParser::HandleEquw },
	{ "SAVE",		&LineParser::HandleSave },
	{ "FOR",		&LineParser::HandleFor },
	{ "NEXT",		&LineParser::HandleNext },
	{ "IF",			&LineParser::HandleIf },
	{ "ELSE",		&LineParser::HandleElse },
	{ "ENDIF",		&LineParser::HandleEndif },
	{ "ALIGN",		&LineParser::HandleAlign },
	{ "SKIP",		&LineParser::HandleSkip },
	{ "GUARD",		&LineParser::HandleGuard },
	{ "CLEAR",		&LineParser::HandleClear },
	{ "SKIPTO",		&LineParser::HandleSkipTo },
	{ "INCBIN",		&LineParser::HandleIncBin },
	{ "{",			&LineParser::HandleOpenBrace },
	{ "}",			&LineParser::HandleCloseBrace }
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

		// create lower-case version of token

		char tokenLC[ 16 ];

		for ( size_t j = 0; j <= len; j++ )
		{
			tokenLC[ j ] = tolower( token[ j ] );
		}

		// see if token matches

		if ( m_line.compare( m_column, len, token ) == 0 ||
			 m_line.compare( m_column, len, tokenLC ) == 0 )
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
	if ( isalpha( m_line[ m_column ] ) || m_line[ m_column ] == '_' )
	{
		// Symbol starts with a valid character

		int oldColumn = m_column;

		// Get the symbol name

		string symbolName = GetSymbolName();

		// ...and mangle it according to whether we are in a FOR loop

		string fullSymbolName = symbolName + m_sourceFile->GetSymbolNameSuffix();

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

	if ( m_line[ m_column ] == ',' )
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

	if ( m_line[ m_column ] == ',' )
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

	if ( m_line[ m_column ] != ',' )
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

	ObjectCode::Instance().Clear( start, end );

	if ( m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
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

	if ( m_line[ m_column ] == ',' )
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

	if ( m_line[ m_column ] == ',' )
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
	if ( addr < 0 || addr > 0xFFFF )
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

	if ( m_line[ m_column ] == ',' )
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
	if ( m_sourceFile->GetForLevel() > 0 )
	{
		// disallow an include within a FOR loop
		throw AsmException_SyntaxError_CantInclude( m_line, m_column );
	}

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != '\"' )
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

		if ( GlobalData::Instance().IsFirstPass() )
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

	if ( m_line[ m_column ] != '\"' )
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

		if ( m_line[ m_column ] == '\"' )
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
					if ( GlobalData::Instance().ShouldOutputAsm() )
					{
						if ( i < 3 )
						{
							cout << setw(2) << static_cast< int >( equs[ i ] ) << " ";
						}
						else if ( i == 3 )
						{
							cout << "...";
						}
					}

					try
					{
						ObjectCode::Instance().PutByte( equs[ i ] );
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
			catch ( AsmException_SyntaxError_SymbolNotDefined& e )
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

		if ( m_line[ m_column ] != ',' )
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
		catch ( AsmException_SyntaxError_SymbolNotDefined& e )
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

		if ( m_line[ m_column ] != ',' )
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

	// syntax is SAVE "filename", start, end [, exec]

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	if ( m_line[ m_column ] != '\"' )
	{
		// did not find a string
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	// get filename

	size_t endQuotePos = m_line.find_first_of( '\"', m_column + 1 );

	if ( endQuotePos == string::npos )
	{
		// did not find the end of the string
		throw AsmException_SyntaxError_MissingQuote( m_line, m_line.length() );
	}

	string saveFile( m_line.substr( m_column + 1, endQuotePos - m_column - 1 ) );

	if ( GlobalData::Instance().ShouldOutputAsm() )
	{
		cout << "Saving file '" << saveFile << "'" << endl;
	}

	m_column = endQuotePos + 1;

	// get start address

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

	start = EvaluateExpressionAsInt();

	if ( start < 0 || start > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	// get end address

	if ( m_line[ m_column ] != ',' )
	{
		// did not find a comma
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	m_column++;

	end = EvaluateExpressionAsInt();

	if ( end < 0 || end > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	// get optional exec address
	// we allow this to be a forward define as it needn't be within the block we actually save

	if ( m_line[ m_column ] == ',' )
	{
		m_column++;

		try
		{
			exec = EvaluateExpressionAsInt();
		}
		catch ( AsmException_SyntaxError_SymbolNotDefined& e )
		{
			if ( GlobalData::Instance().IsSecondPass() )
			{
				throw;
			}
		}
	}
	else
	{
		exec = start;
	}

	if ( exec < 0 || exec > 0xFFFF )
	{
		throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
	}

	// expect no more

	if ( AdvanceAndCheckEndOfStatement() )
	{
		// found something else - wrong!
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	// OK - do it

	if ( GlobalData::Instance().IsSecondPass() )
	{
		if ( GlobalData::Instance().UsesDiscImage() )
		{
			// disc image version of the save
			GlobalData::Instance().GetDiscImage()->AddFile( saveFile.c_str(),
															ObjectCode::Instance().GetAddr( start ),
															start,
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
	string symbolName = GetSymbolName() + m_sourceFile->GetSymbolNameSuffix();

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

	m_sourceFile->AddFor( symbolName,
						  start, end, step,
						  m_sourceFile->GetFilePointer() + m_column,
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
	m_sourceFile->OpenBrace( m_line, m_column - 1 );
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

	m_sourceFile->UpdateFor( m_line, oldColumn );
}



/*************************************************************************************************/
/**
	LineParser::HandleCloseBrace()

	Braces for scoping variables are just FORs in disguise...
*/
/*************************************************************************************************/
void LineParser::HandleCloseBrace()
{
	m_sourceFile->CloseBrace( m_line, m_column - 1 );
}



/*************************************************************************************************/
/**
	LineParser::HandleIf()
*/
/*************************************************************************************************/
void LineParser::HandleIf()
{
	int condition = EvaluateExpressionAsInt();
	m_sourceFile->SetCurrentIfCondition( condition );

	if ( m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleElse()
*/
/*************************************************************************************************/
void LineParser::HandleElse()
{
	if ( AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleEndif()
*/
/*************************************************************************************************/
void LineParser::HandleEndif()
{
	if ( AdvanceAndCheckEndOfStatement() )
	{
		// found nothing
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
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
			catch ( AsmException_SyntaxError_SymbolNotDefined& e )
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
			catch ( AsmException_SyntaxError_SymbolNotDefined& e )
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
