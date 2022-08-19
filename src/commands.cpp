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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>

#include "lineparser.h"
#include "globaldata.h"
#include "objectcode.h"
#include "stringutils.h"
#include "symboltable.h"
#include "sourcefile.h"
#include "asmexception.h"
#include "discimage.h"
#include "BASIC.h"
#include "random.h"


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
	{ "PUTTEXT",	&LineParser::HandlePutText,				0 },
	{ "PUTBASIC",	&LineParser::HandlePutBasic,			0 },
	{ "MACRO",		&LineParser::HandleMacro,				&SourceFile::StartMacro },
	{ "ENDMACRO",	&LineParser::HandleEndMacro,			&SourceFile::EndMacro },
	{ "ERROR",		&LineParser::HandleError,				0 },
	{ "COPYBLOCK",	&LineParser::HandleCopyBlock,			0 },
	{ "RANDOMIZE",  &LineParser::HandleRandomize,			0 },
	{ "ASM",		&LineParser::HandleAsm,					0 }
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
	size_t remaining = m_line.length() - m_column;

	for ( int i = 0; i < static_cast<int>( sizeof m_gaTokenTable / sizeof( Token ) ); i++ )
	{
		const char*	token	= m_gaTokenTable[ i ].m_pName;
		size_t		len		= strlen( token );

		if (len <= remaining)
		{
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
	}

	return -1;
}



// Argument represents an attempt to parse an argument of type T from an argument list.
// Errors are mostly not thrown until the value is realised because at the time of
// parsing we don't know if the the value is optional or allowed to be undefined.
// For example, an optional string shouldn't immediately throw an error if it sees
// an undefined symbol because that may fit the next parameter.
template<class T> class Argument
{
public:
	typedef T ContainedType;

	enum State
	{
		// A value of type T was successfully parsed
		StateFound,
		// A value of the wrong type was available
		StateTypeMismatch,
		// A symbol is undefined
		StateUndefined,
		// No value was available (i.e. the end of the parameters)
		StateMissing
	};
	Argument(string line, int column, State state) :
		m_line(line), m_column(column), m_state(state)
	{
	}
	Argument(string line, int column, T value) :
		m_line(line), m_column(column), m_state(StateFound), m_value(value)
	{
	}
	Argument(const Argument<T>& that) :
		m_line(that.m_line), m_column(that.m_column), m_state(that.m_state), m_value(that.m_value)
	{
		m_line = that.m_line;
		m_column = that.m_column;
		m_state = that.m_state;
		m_value = that.m_value;
	}
	// Is a value available?
	bool Found() const
	{
		return m_state == StateFound;
	}
	// The column at which the parameter started.
	int Column() const
	{
		return m_column;
	}
	// Extract the parameter value, throwing an exception if it didn't exist.
	operator T() const
	{
		switch (m_state)
		{
		case StateFound:
			return m_value;
		case StateTypeMismatch:
			throw AsmException_SyntaxError_TypeMismatch( m_line, m_column );
		case StateUndefined:
			throw AsmException_SyntaxError_SymbolNotDefined( m_line, m_column );
		case StateMissing:
		default:
			throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
		}
	}
	// Check the parameter lies within a range.
	Argument<T>& Range(T mn, T mx)
	{
		if ( Found() && ( mn > m_value || m_value > mx ) )
		{
			throw AsmException_SyntaxError_OutOfRange( m_line, m_column );
		}
		return *this;
	}
	// Check the parameter does not exceed a maximum.
	Argument<T>& Maximum(T mx)
	{
		if ( Found() && m_value > mx )
		{
			throw AsmException_SyntaxError_NumberTooBig( m_line, m_column );
		}
		return *this;
	}
	// Set a default value for optional parameters.
	// This is overloaded for strings below.
	Argument<T>& Default(T value)
	{
		if ( !Found() )
		{
			if ( m_state == StateUndefined)
			{
				throw AsmException_SyntaxError_SymbolNotDefined( m_line, m_column );
			}
			m_value = value;
			m_state = StateFound;
		}
		return *this;
	}
	// Permit this parameter to be an undefined symbol.
	// This is overloaded for strings below.
	Argument<T>& AcceptUndef()
	{
		if (m_state == StateUndefined)
		{
			m_state = StateFound;
			m_value = 0;
		}
		return *this;
	}
private:
	// Prevent assignment
	Argument<T> operator=(const Argument<T>& that);

	string m_line;
	int m_column;
	State m_state;
	T m_value;
};

typedef Argument<int> IntArg;
typedef Argument<double> DoubleArg;
typedef Argument<string> StringArg;
typedef Argument<Value> ValueArg;

template<> StringArg& StringArg::Default(string value)
{
	if ( !Found() )
	{
		m_value = value;
		m_state = StateFound;
	}
	return *this;
}

// AcceptUndef should not be called for string types.
template<> StringArg& StringArg::AcceptUndef()
{
	assert(false);
	if (m_state == StateUndefined)
	{
		throw AsmException_SyntaxError_SymbolNotDefined( m_line, m_column );
	}
	return *this;
}


// ArgListParser helps parse a list of arguments.
class ArgListParser
{
public:
	ArgListParser(LineParser& lineParser, bool comma_first = false) : m_lineParser(lineParser)
	{
		m_first = !comma_first;
		m_pending = false;
	}

	IntArg ParseInt()
	{
		return ParseNumber<IntArg>(&ArgListParser::ConvertDoubleToInt);
	}

	DoubleArg ParseDouble()
	{
		return ParseNumber<DoubleArg>(&ArgListParser::ConvertDoubleToDouble);
	}

	StringArg ParseString()
	{
		if ( !ReadPending() )
		{
			return StringArg(m_lineParser.m_line, m_paramColumn, StringArg::StateMissing);
		}
		if ( m_pendingUndefined )
		{
			return StringArg(m_lineParser.m_line, m_paramColumn, StringArg::StateUndefined);
		}
		if ( m_pendingValue.GetType() != Value::StringValue )
		{
			return StringArg(m_lineParser.m_line, m_paramColumn, StringArg::StateTypeMismatch);
		}
		m_pending = false;
		String temp = m_pendingValue.GetString();
		return StringArg(m_lineParser.m_line, m_paramColumn, string(temp.Text(), temp.Length()));
	}

	ValueArg ParseValue()
	{
		if ( !ReadPending() )
		{
			return ValueArg(m_lineParser.m_line, m_paramColumn, ValueArg::StateMissing);
		}
		if ( m_pendingUndefined )
		{
			m_pending = false;
			return ValueArg(m_lineParser.m_line, m_paramColumn, StringArg::StateUndefined);
		}
		m_pending = false;
		return ValueArg(m_lineParser.m_line, m_paramColumn, m_pendingValue);
	}

	void CheckComplete()
	{
		if ( m_pending )
		{
			throw AsmException_SyntaxError_TypeMismatch( m_lineParser.m_line, m_lineParser.m_column );
		}
		if ( m_lineParser.AdvanceAndCheckEndOfStatement() )
		{
			throw AsmException_SyntaxError_InvalidCharacter( m_lineParser.m_line, m_lineParser.m_column );
		}
	}
private:
	// Prevent copies
	ArgListParser(const ArgListParser& that);
	ArgListParser operator=(const ArgListParser& that);

	int ConvertDoubleToInt(double value)
	{
		return m_lineParser.ConvertDoubleToInt(value);
	}

	double ConvertDoubleToDouble(double value)
	{
		return value;
	}

	template <class T> T ParseNumber(typename T::ContainedType (ArgListParser::*convertDoubleTo)(double))
	{
		if ( !ReadPending() )
		{
			return T(m_lineParser.m_line, m_paramColumn, T::StateMissing);
		}
		if ( m_pendingUndefined )
		{
			m_pending = false;
			return T(m_lineParser.m_line, m_paramColumn, T::StateUndefined);
		}
		if ( m_pendingValue.GetType() != Value::NumberValue )
		{
			return T(m_lineParser.m_line, m_paramColumn, T::StateTypeMismatch);
		}
		m_pending = false;
		return T(m_lineParser.m_line, m_paramColumn, (this->*convertDoubleTo)(m_pendingValue.GetNumber()));
	}

	// Return true if an argument is available
	bool ReadPending()
	{
		if (!m_pending)
		{
			bool found = MoveNext();
			m_paramColumn = m_lineParser.m_column;
			if (found)
			{
				try
				{
					m_pendingUndefined = false;
					m_pendingValue = m_lineParser.EvaluateExpression();
				}
				catch ( AsmException_SyntaxError_SymbolNotDefined& )
				{
					if ( !GlobalData::Instance().IsFirstPass() )
					{
						throw;
					}
					m_pendingUndefined = true;
					m_pendingValue = 0;
				}
				m_pending = true;
			}
		}
		return m_pending;
	}

	// Return true if there's something to parse
	bool MoveNext()
	{
		if (m_first)
		{
			m_first = false;
			return m_lineParser.AdvanceAndCheckEndOfStatement();
		}
		else
		{
			if ( m_lineParser.AdvanceAndCheckEndOfStatement() )
			{
				// If there's anything of interest it must be a comma
				if ( m_lineParser.m_column >= m_lineParser.m_line.length() || m_lineParser.m_line[ m_lineParser.m_column ] != ',' )
				{
					// did not find a comma
					throw AsmException_SyntaxError_InvalidCharacter( m_lineParser.m_line, m_lineParser.m_column );
				}
				m_lineParser.m_column++;
				StringUtils::EatWhitespace( m_lineParser.m_line, m_lineParser.m_column );
				return true;
			}
			return false;
		}
	}

	LineParser& m_lineParser;
	int m_paramColumn;
	bool m_first;
	bool m_pending;
	bool m_pendingUndefined;
	Value m_pendingValue;
};



/*************************************************************************************************/
/**
	LineParser::HandleDefineLabel()
*/
/*************************************************************************************************/
void LineParser::HandleDefineLabel()
{
        if ( m_column >= m_line.length() )
	{
		throw AsmException_SyntaxError_InvalidSymbolName( m_line, m_column );
	}

        int initialColumn = m_column;
        char first_char = m_line[ m_column ];
        int target_level = m_sourceCode->GetForLevel();
        if (first_char == '*')
        {
                m_column++;
                target_level = 0;
        }
        else if (first_char == '^')
        {
                m_column++;
                target_level = std::max( target_level - 1, 0 );
        }

        // '*' and '^' may not cause a label to be defined outside the current macro expansion.
        if ( target_level < m_sourceCode->GetInitialForStackPtr() )
        {
                throw AsmException_SyntaxError_SymbolScopeOutsideMacro( m_line, initialColumn );
        }

        // '*' and '^' may not cause a label to be defined outside the current for loop; note that
        // this loop is a no-op for ordinary labels where target_level == m_sourceCode->GetForLevel().
        for ( int level = m_sourceCode->GetForLevel(); level > target_level; level-- )
        {
                if ( m_sourceCode->IsRealForLevel( level ) )
                {
                        throw AsmException_SyntaxError_SymbolScopeOutsideFor( m_line, initialColumn );
                }
        }

	if ( ( m_column < m_line.length() ) && ( isalpha( m_line[ m_column ] ) || m_line[ m_column ] == '_' ) )
	{
		// Symbol starts with a valid character

		int oldColumn = m_column;

		// Get the symbol name

		string symbolName = GetSymbolName();

		// ...and mangle it according to whether we are in a FOR loop

		string fullSymbolName = symbolName + m_sourceCode->GetSymbolNameSuffix( target_level );

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
			// on the second pass, check that the label would be assigned the same numeric value

			Value value = SymbolTable::Instance().GetSymbol( fullSymbolName );
			if ((value.GetType() != Value::NumberValue) || (value.GetNumber() != ObjectCode::Instance().GetPC() ))
			{
				throw AsmException_SyntaxError_SecondPassProblem( m_line, oldColumn );
			}

			SymbolTable::Instance().AddLabel(symbolName);
		}

		if ( m_sourceCode->ShouldOutputAsm() )
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
	ArgListParser args(*this);
	int newPC = args.ParseInt().Range(0, 0xFFFF);
	args.CheckComplete();

	ObjectCode::Instance().SetPC( newPC );
	SymbolTable::Instance().ChangeSymbol( "P%", newPC );
}



/*************************************************************************************************/
/**
	LineParser::HandleCpu()
*/
/*************************************************************************************************/
void LineParser::HandleCpu()
{
	ArgListParser args(*this);
	int newCpu = args.ParseInt().Range(0, 1);
	args.CheckComplete();

	ObjectCode::Instance().SetCPU( newCpu );
}



/*************************************************************************************************/
/**
	LineParser::HandleGuard()
*/
/*************************************************************************************************/
void LineParser::HandleGuard()
{
	ArgListParser args(*this);
	int val = args.ParseInt().Range(0, 0xFFFF);
	args.CheckComplete();

	ObjectCode::Instance().SetGuard( val );
}



/*************************************************************************************************/
/**
	LineParser::HandleClear()
*/
/*************************************************************************************************/
void LineParser::HandleClear()
{
	ArgListParser args(*this);

	int start = args.ParseInt().Range(0, 0xFFFF);
	int end  = args.ParseInt().Range(0, 0x10000);

	args.CheckComplete();

	ObjectCode::Instance().Clear( start, end );
}



/*************************************************************************************************/
/**
	LineParser::HandleMapChar()
*/
/*************************************************************************************************/
void LineParser::HandleMapChar()
{
	// get parameters - either 2 or 3

	ArgListParser args(*this);

	int param1 = args.ParseInt().Range(0x20, 0x7E);
	int param2 = args.ParseInt().Range(0, 0xFF);
	IntArg param3 = args.ParseInt().Range(0, 0xFF);

	args.CheckComplete();

	if ( !param3.Found() )
	{
		// two parameters

		// do single character remapping
		ObjectCode::Instance().SetMapping( param1, param2 );
	}
	else
	{
		// three parameters

		if ( param2 < 0x20 || param2 > 0x7E || param2 < param1 )
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

	if ( m_sourceCode->ShouldOutputAsm() )
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
	ArgListParser args(*this);
	IntArg addr = args.ParseInt().Range(0, 0x10000);
	args.CheckComplete();

	if ( ObjectCode::Instance().GetPC() > addr )
	{
		throw AsmException_SyntaxError_BackwardsSkip( m_line, addr.Column() );
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

	string filename = EvaluateExpressionAsString();

	if ( m_sourceCode->ShouldOutputAsm() )
	{
		cerr << "Including file " << filename << endl;
	}

	SourceFile input( filename.c_str(), m_sourceCode );
	input.Process();

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
	string filename = EvaluateExpressionAsString();

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
	ArgListParser args(*this);

	Value value = args.ParseValue().AcceptUndef();

	do
	{
		if (value.GetType() == Value::StringValue)
		{
			// handle equs
			HandleEqus( value.GetString() );
		}
		else if (value.GetType() == Value::NumberValue)
		{
			// handle byte
			int number = static_cast<int>(value.GetNumber());

			if ( number > 0xFF )
			{
				throw AsmException_SyntaxError_NumberTooBig( m_line, m_column );
			}

			if ( m_sourceCode->ShouldOutputAsm() )
			{
				cout << uppercase << hex << setfill( '0' ) << "     ";
				cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
				cout << setw(2) << ( number & 0xFF );
				cout << endl << nouppercase << dec << setfill( ' ' );
			}

			try
			{
				ObjectCode::Instance().PutByte( number & 0xFF );
			}
			catch ( AsmException_AssembleError& e )
			{
				e.SetString( m_line );
				e.SetColumn( m_column );
				throw;
			}
		}
		else
		{
			// Unknown value type; this should never happen.
			assert(false);
		}

		ValueArg arg = args.ParseValue().AcceptUndef();
		if (!arg.Found())
			break;

		value = arg;

	} while ( true );

	args.CheckComplete();
}



/*************************************************************************************************/
/**
	LineParser::HandleEqus( const String& equs )
*/
/*************************************************************************************************/
void LineParser::HandleEqus( const String& equs )
{
	if ( m_sourceCode->ShouldOutputAsm() )
	{
		cout << uppercase << hex << setfill( '0' ) << "     ";
		cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
	}

	for ( size_t i = 0; i < equs.Length(); i++ )
	{
		int mappedchar = ObjectCode::Instance().GetMapping( equs[ i ] );

		if ( m_sourceCode->ShouldOutputAsm() )
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

	if ( m_sourceCode->ShouldOutputAsm() )
	{
		cout << endl << nouppercase << dec << setfill( ' ' );
	}
}



/*************************************************************************************************/
/**
	LineParser::HandleEquw()
*/
/*************************************************************************************************/
void LineParser::HandleEquw()
{
	ArgListParser args(*this);

	int value = args.ParseInt().AcceptUndef().Maximum(0xFFFF);

	do
	{
		if ( m_sourceCode->ShouldOutputAsm() )
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

		IntArg arg = args.ParseInt().AcceptUndef().Maximum(0xFFFF);
		if (!arg.Found())
			break;

		value = arg;

	} while ( true );

	args.CheckComplete();
}



/*************************************************************************************************/
/**
	LineParser::HandleEqud()
*/
/*************************************************************************************************/
void LineParser::HandleEqud()
{
	ArgListParser args(*this);

	int value = args.ParseInt().AcceptUndef();

	do
	{
		if ( m_sourceCode->ShouldOutputAsm() )
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

		IntArg arg = args.ParseInt().AcceptUndef();
		if (!arg.Found())
			break;

		value = arg;

	} while ( true );

	args.CheckComplete();
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
	// syntax is SAVE ["filename"], start, end [, exec [, reload] ]

	ArgListParser args(*this);

	StringArg saveParam = args.ParseString();
	int start = args.ParseInt().Range(0, 0xFFFF);
	int end = args.ParseInt().Range(0, 0x10000);
	int exec = args.ParseInt().AcceptUndef().Default(start).Range(0, 0xFFFFFF);
	int reload = args.ParseInt().Default(start).Range(0, 0xFFFFFF);
	args.CheckComplete();

	if ( !saveParam.Found() )
	{
		if ( GlobalData::Instance().GetOutputFile() != NULL )
		{
			saveParam.Default(GlobalData::Instance().GetOutputFile());

			if ( GlobalData::Instance().IsSecondPass() )
			{
				if ( GlobalData::Instance().GetNumAnonSaves() > 0 )
				{
					throw AsmException_SyntaxError_OnlyOneAnonSave( m_line, saveParam.Column() );
				}
				else
				{
					GlobalData::Instance().IncNumAnonSaves();
				}
			}
		}
		else
		{
			throw AsmException_SyntaxError_NoAnonSave( m_line, saveParam.Column() );
		}
	}

	string saveFile = saveParam;

	if ( m_sourceCode->ShouldOutputAsm() )
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

	ArgListParser args(*this, true);
	double start = args.ParseDouble();
	double end = args.ParseDouble();
	double step = args.ParseDouble().Default(1);
	args.CheckComplete();

	if ( step == 0.0 )
	{
		throw AsmException_SyntaxError_BadStep( m_line, m_column );
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
			StringUtils::EatWhitespace( m_line, m_column );
			const char* filelineKeyword = "FILELINE$";
			const int filelineKeywordLength = 9;
			const char* callstackKeyword = "CALLSTACK$";
			const int callstackKeywordLength = 10;

			if ( !strncmp( m_line.c_str() + m_column, filelineKeyword, filelineKeywordLength ) )
			{
				if ( !GlobalData::Instance().IsFirstPass() )
				{
					cout << StringUtils::FormattedErrorLocation( m_sourceCode->GetFilename(), m_sourceCode->GetLineNumber() );
				}
				m_column += filelineKeywordLength ;
			}
			else if ( !strncmp( m_line.c_str() + m_column, callstackKeyword, callstackKeywordLength ) )
			{
				if ( !GlobalData::Instance().IsFirstPass() )
				{
					cout << StringUtils::FormattedErrorLocation( m_sourceCode->GetFilename(), m_sourceCode->GetLineNumber() );
					for ( const SourceCode* s = m_sourceCode->GetParent(); s; s = s->GetParent() )
					{
						cout << endl << StringUtils::FormattedErrorLocation( s->GetFilename(), s->GetLineNumber() );
					}
				}
				m_column += callstackKeywordLength;
			}
			else
			{
				// print number in decimal or string

				Value value;

				try
				{
					value = EvaluateExpression();
				}
				catch ( AsmException_SyntaxError_SymbolNotDefined& )
				{
					if ( GlobalData::Instance().IsSecondPass() )
					{
						throw;
					}
				}

				if ( GlobalData::Instance().IsSecondPass() )
				{
					if (value.GetType() == Value::NumberValue)
					{
						StringUtils::PrintNumber(cout, value.GetNumber());
						cout << " ";
					}
					else if (value.GetType() == Value::StringValue)
					{
						String text = value.GetString();
						const char* pstr = text.Text();
						for (unsigned int i = 0; i != text.Length(); ++i)
						{
							cout << *pstr;
							++pstr;
						}
					}
				}
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
	LineParser::HandlePutText()
*/
/*************************************************************************************************/
void LineParser::HandlePutText()
{
	HandlePutFileCommon(true);
}



/*************************************************************************************************/
/**
	LineParser::HandlePutFile()
*/
/*************************************************************************************************/
void LineParser::HandlePutFile()
{
	HandlePutFileCommon(false);
}



/*************************************************************************************************/
/**
	LineParser::HandlePutFileCommon()
*/
/*************************************************************************************************/
void LineParser::HandlePutFileCommon( bool bText )
{
	// Syntax:
	// PUTFILE/PUTTEXT <host filename>, [<beeb filename>,] <start addr> [,<exec addr>]

	ArgListParser args(*this);

	string hostFilename = args.ParseString();
	string beebFilename = args.ParseString().Default(hostFilename);
	int start = args.ParseInt().AcceptUndef().Range(0, 0xFFFFFF);
	int exec = args.ParseInt().AcceptUndef().Default(start).Range(0, 0xFFFFFF);

	args.CheckComplete();

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
		if ( bText )
		{
			fileSize = 0;
			int c;
			while ( ( c = inputFile.get() ) != EOF )
			{
				if ( c == '\n' || c == '\r' )
				{
					// swallow other half of CRLF/LFCR, if present
					int other_half = ( c == '\n' ) ? '\r' : '\n';
					std::streampos p = inputFile.tellg();
					if ( inputFile.get() != other_half )
					{
						inputFile.seekg( p );
					}

					buffer[ fileSize ] = '\r';
				}
				else
				{
					buffer[ fileSize ] = c;
				}
				++fileSize;
			}
		}
		else
		{
			inputFile.read( buffer, fileSize );
		}
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
	string hostFilename = EvaluateExpressionAsString();
	string beebFilename = hostFilename;

	if ( AdvanceAndCheckEndOfStatement() )
	{
		// see if there's a second parameter

		if ( m_line[ m_column ] != ',' )
		{
			throw AsmException_SyntaxError_MissingComma( m_line, m_column );
		}

		m_column++;

		beebFilename = EvaluateExpressionAsString();
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

	string errorMsg = EvaluateExpressionAsString();

	// This is a slight change in behaviour.  It used to check the statement
	// was well-formed after the error was thrown.
	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	// throw error
	throw AsmException_UserError( m_line, oldColumn, errorMsg );
}


/*************************************************************************************************/
/**
	LineParser::HandleCopyBlock()
*/
/*************************************************************************************************/
void LineParser::HandleCopyBlock()
{
	ArgListParser args(*this);

	int start = args.ParseInt().Range(0, 0xFFFF);
	int end = args.ParseInt().Range(0, 0xFFFF);
	int dest = args.ParseInt().Range(0, 0xFFFF);

	args.CheckComplete();

	try
	{
		ObjectCode::Instance().CopyBlock( start, end, dest );
	}
	catch ( AsmException_AssembleError& e )
	{
		e.SetString( m_line );
		e.SetColumn( m_column );
		throw;
	}
}


/*************************************************************************************************/
/**
	LineParser::HandleRandomize()
*/
/*************************************************************************************************/
void LineParser::HandleRandomize()
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

	beebasm_srand( value );

	if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
	{
		// Unexpected comma (remembering that an expression can validly end with a comma)
		throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
	}
}


/*************************************************************************************************/
/**
	LineParser::HandleAsm()
*/
/*************************************************************************************************/
void LineParser::HandleAsm()
{
	// look for assembly language string

	string assembly = EvaluateExpressionAsString();

	// check this is now the end

	if ( AdvanceAndCheckEndOfStatement() )
	{
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	LineParser parser(m_sourceCode, assembly);

	// Parse the mnemonic, don't require a non-alpha after it.
	int instruction = parser.GetInstructionAndAdvanceColumn(false);
	if (instruction < 0)
	{
		throw AsmException_SyntaxError_MissingAssemblyInstruction( parser.m_line, parser.m_column );
	}

	parser.HandleAssembler(instruction);
}
