/*************************************************************************************************/
/**
	expression.cpp

	Contains all the LineParser methods for parsing and evaluating expressions


	Copyright (C) Rich Talbot-Watkins 2007, 2008

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

#include <cmath>
#include <cerrno>
#include <sstream>
#include <iomanip>

#include "lineparser.h"
#include "asmexception.h"
#include "symboltable.h"
#include "globaldata.h"
#include "objectcode.h"
#include "sourcefile.h"


using namespace std;



LineParser::Operator	LineParser::m_gaBinaryOperatorTable[] =
{
	{ ")",		-1,	NULL },		// special case
	{ "]",		-1,	NULL },		// special case

	{ "^",		7,	&LineParser::EvalPower },
	{ "*",		6,	&LineParser::EvalMultiply },
	{ "/",		6,	&LineParser::EvalDivide },
	{ "%",		6,	&LineParser::EvalMod },
	{ "DIV",	6,	&LineParser::EvalDiv },
	{ "MOD",	6,	&LineParser::EvalMod },
	{ "<<",		6,	&LineParser::EvalShiftLeft },
	{ ">>",		6,	&LineParser::EvalShiftRight },
	{ "+",		5,	&LineParser::EvalAdd },
	{ "-",		5,	&LineParser::EvalSubtract },
	{ "==",		4,	&LineParser::EvalEqual },
	{ "=",		4,	&LineParser::EvalEqual },
	{ "<>",		4,	&LineParser::EvalNotEqual },
	{ "!=",		4,	&LineParser::EvalNotEqual },
	{ "<=",		4,	&LineParser::EvalLessThanOrEqual },
	{ ">=",		4,	&LineParser::EvalMoreThanOrEqual },
	{ "<",		4,	&LineParser::EvalLessThan },
	{ ">",		4,	&LineParser::EvalMoreThan },
	{ "AND",	3,	&LineParser::EvalAnd },
	{ "OR",		2,	&LineParser::EvalOr },
	{ "EOR",	2,	&LineParser::EvalEor }
};



LineParser::Operator	LineParser::m_gaUnaryOperatorTable[] =
{
	{ "(",		-1,	NULL },		// special case
	{ "[",		-1,	NULL },		// special case

	{ "-",		8,	&LineParser::EvalNegate },
	{ "+",		8,	&LineParser::EvalPosate },
	{ "HI",		10,	&LineParser::EvalHi },
	{ "LO",		10,	&LineParser::EvalLo },
	{ ">",		10,	&LineParser::EvalHi },
	{ "<",		10,	&LineParser::EvalLo },
	{ "SIN",	10, &LineParser::EvalSin },
	{ "COS",	10, &LineParser::EvalCos },
	{ "TAN",	10, &LineParser::EvalTan },
	{ "ASN",	10, &LineParser::EvalArcSin },
	{ "ACS",	10, &LineParser::EvalArcCos },
	{ "ATN",	10, &LineParser::EvalArcTan },
	{ "SQR",	10, &LineParser::EvalSqrt },
	{ "RAD",	10, &LineParser::EvalDegToRad },
	{ "DEG",	10, &LineParser::EvalRadToDeg },
	{ "INT",	10,	&LineParser::EvalInt },
	{ "ABS",	10, &LineParser::EvalAbs },
	{ "SGN",	10, &LineParser::EvalSgn },
	{ "RND",	10,	&LineParser::EvalRnd },
	{ "NOT",	10, &LineParser::EvalNot },
	{ "LOG",	10, &LineParser::EvalLog },
	{ "LN",		10,	&LineParser::EvalLn },
	{ "EXP",	10,	&LineParser::EvalExp }
};



/*************************************************************************************************/
/**
	LineParser::GetValue()

	Parses a simple value.  This may be
	- a decimal literal
	- a hex literal (prefixed by &)
	- a symbol (label)
	- a special value such as * (PC)

	@return		double
*/
/*************************************************************************************************/
double LineParser::GetValue()
{
	double value = 0;

	if ( isdigit( m_line[ m_column ] ) || m_line[ m_column ] == '.' )
	{
		// get a number

		istringstream str( m_line );
		str.seekg( m_column );
		str >> value;
		m_column = str.tellg();
	}
	else if ( m_line[ m_column ] == '&' || m_line[ m_column ] == '$' )
	{
		// get a hex digit

		m_column++;

		if ( !isxdigit( m_line[ m_column ] ) )
		{
			// badly formed hex literal
			throw AsmException_SyntaxError_BadHex( m_line, m_column );
		}
		else
		{
			// get a number

			int hexValue;

			istringstream str( m_line );
			str.seekg( m_column );
			str >> hex >> hexValue;
			m_column = str.tellg();

			value = static_cast< double >( hexValue );
		}
	}
	else if ( m_line[ m_column ] == '%' )
	{
		// get binary

		m_column++;

		if ( m_line[ m_column ] != '0' && m_line[ m_column ] != '1' )
		{
			// badly formed bin literal
			throw AsmException_SyntaxError_BadBin( m_line, m_column );
		}
		else
		{
			// parse binary number

			int binValue = 0;

			do
			{
				binValue = ( binValue * 2 ) + ( m_line[ m_column ] - '0' );
				m_column++;
			}
			while ( m_line[ m_column ] == '0' || m_line[ m_column ] == '1' );

			value = static_cast< double >( binValue );
		}
	}
	else if ( m_line[ m_column ] == '*' )
	{
		// get current PC

		m_column++;
		value = static_cast< double >( ObjectCode::Instance().GetPC() );
	}
	else if ( m_line[ m_column ] == '\'' )
	{
		// get char literal

		if ( m_line.length() - m_column < 3 ||  m_line[ m_column + 2 ] != '\'' )
		{
			// bad syntax - must be e.g. 'A'
			throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
		}

		value = static_cast< double >( m_line[ m_column + 1 ] );
		m_column += 3;
	}
	else if ( isalpha( m_line[ m_column ] ) || m_line[ m_column ] == '_' )
	{
		// get a symbol

		int oldColumn = m_column;
		string symbolName = GetSymbolName();
		bool bFoundSymbol = false;

		for ( int forLevel = m_sourceFile->GetForLevel(); forLevel >= 0; forLevel-- )
		{
			string fullSymbolName = symbolName + m_sourceFile->GetSymbolNameSuffix( forLevel );

			if ( SymbolTable::Instance().IsSymbolDefined( fullSymbolName ) )
			{
				value = SymbolTable::Instance().GetSymbol( fullSymbolName );
				bFoundSymbol = true;
				break;
			}
		}

		if ( !bFoundSymbol )
		{
			// symbol not known
			throw AsmException_SyntaxError_SymbolNotDefined( m_line, oldColumn );
		}
	}
	else
	{
		// expected value
		throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
	}

	return value;
}



/*************************************************************************************************/
/**
	LineParser::EvaluateExpression()

	Evaluates an expression, and returns its value, also advancing the string pointer
*/
/*************************************************************************************************/
double LineParser::EvaluateExpression( bool bAllowOneMismatchedCloseBracket )
{
	// Reset stacks

	m_valueStackPtr = 0;
	m_operatorStackPtr = 0;

	// Count brackets

	int bracketCount = 0;

	TYPE expected = VALUE_OR_UNARY;

	// Iterate through the expression

	while ( AdvanceAndCheckEndOfSubStatement() )
	{
		if ( expected == VALUE_OR_UNARY )
		{
			// Look for unary operator

			int matchedToken = -1;

			// Check against unary operator tokens

			for ( unsigned int i = 0; i < sizeof m_gaUnaryOperatorTable / sizeof(Operator); i++ )
			{
				const char*		token	= m_gaUnaryOperatorTable[ i ].token;
				size_t			len		= strlen( token );

				// see if token matches

				if ( m_line.compare( m_column, len, token ) == 0 )
				{
					matchedToken = i;
					m_column += len;
					break;
				}
			}

			if ( matchedToken == -1 )
			{
				// If unary operator not found, look for a value instead

				if ( m_valueStackPtr == MAX_VALUES )
				{
					throw AsmException_SyntaxError_ExpressionTooComplex( m_line, m_column );
				}

				double value;

				try
				{
					value = GetValue();
				}
				catch ( AsmException_SyntaxError_SymbolNotDefined& e )
				{
					// If we encountered an unknown symbol whilst evaluating the expression...

					if ( GlobalData::Instance().IsFirstPass() )
					{
						// On first pass, we have to continue gracefully.
						// This moves the string pointer to beyond the expression

						SkipExpression( bracketCount, bAllowOneMismatchedCloseBracket );
					}

					// Whatever happens, we throw the exception
					throw;
				}

				m_valueStack[ m_valueStackPtr++ ] = value;
				expected = BINARY;
			}
			else
			{
				// If unary operator *was* found...

				Operator& thisOp = m_gaUnaryOperatorTable[ matchedToken ];

				if ( thisOp.handler != NULL )
				{
					// not an open bracket - we may have to juggle the stack

					while ( m_operatorStackPtr > 0 &&
							thisOp.precedence < m_operatorStack[ m_operatorStackPtr - 1 ].precedence )
					{
						m_operatorStackPtr--;

						OperatorHandler opHandler = m_operatorStack[ m_operatorStackPtr ].handler;
						assert( opHandler != NULL );	// this should really not be possible!

						( this->*opHandler )();
					}
				}
				else
				{
					bracketCount++;
				}

				if ( m_operatorStackPtr == MAX_OPERATORS )
				{
					throw AsmException_SyntaxError_ExpressionTooComplex( m_line, m_column );
				}

				m_operatorStack[ m_operatorStackPtr++ ] = thisOp;
			}

		}
		else
		{
			// Get binary operator

			int matchedToken = -1;

			for ( unsigned int i = 0; i < sizeof m_gaBinaryOperatorTable / sizeof(Operator); i++ )
			{
				const char*		token	= m_gaBinaryOperatorTable[ i ].token;
				size_t			len		= strlen( token );

				// see if token matches

				if ( m_line.compare( m_column, len, token ) == 0 )
				{
					matchedToken = i;
					m_column += len;
					break;
				}
			}

			if ( matchedToken == -1 )
			{
				// unrecognised binary op
				throw AsmException_SyntaxError_InvalidCharacter( m_line, m_column );
			}

			// we found binary operator

			Operator& thisOp = m_gaBinaryOperatorTable[ matchedToken ];

			if ( thisOp.handler != NULL )
			{
				// not an close bracket

				while ( m_operatorStackPtr > 0 &&
						thisOp.precedence <= m_operatorStack[ m_operatorStackPtr - 1 ].precedence )
				{
					m_operatorStackPtr--;

					OperatorHandler opHandler = m_operatorStack[ m_operatorStackPtr ].handler;
					assert( opHandler != NULL );	// this means the operator has been given a precedence of < 0

					( this->*opHandler )();
				}

				if ( m_operatorStackPtr == MAX_OPERATORS )
				{
					throw AsmException_SyntaxError_ExpressionTooComplex( m_line, m_column );
				}

				m_operatorStack[ m_operatorStackPtr++ ] = thisOp;

				expected = VALUE_OR_UNARY;
			}
			else
			{
				// is a close bracket

				bracketCount--;

				bool bFoundMatchingBracket = false;

				while ( m_operatorStackPtr > 0 )
				{
					m_operatorStackPtr--;

					OperatorHandler opHandler = m_operatorStack[ m_operatorStackPtr ].handler;
					if ( opHandler != NULL )
					{
						( this->*opHandler )();
					}
					else
					{
						bFoundMatchingBracket = true;
						break;
					}
				}

				if ( !bFoundMatchingBracket )
				{
					if ( bAllowOneMismatchedCloseBracket )
					{
						// this is a hack which allows an extra close bracket to terminate an expression,
						// so that we can parse LDA (ind),Y and JMP (ind) where the open bracket is not
						// included in the expression

						m_column--;
						break;	// jump out of the loop, ready to exit
					}
					else
					{
						// mismatched brackets
						throw AsmException_SyntaxError_MismatchedParentheses( m_line, m_column - 1 );
					}
				}
			}
		}
	}

	// purge the operator stack

	while ( m_operatorStackPtr > 0 )
	{
		m_operatorStackPtr--;

		OperatorHandler opHandler = m_operatorStack[ m_operatorStackPtr ].handler;

		if ( opHandler == NULL )
		{
			// mismatched brackets
			throw AsmException_SyntaxError_MismatchedParentheses( m_line, m_column );
		}
		else
		{
			( this->*opHandler )();
		}
	}

	assert( m_valueStackPtr <= 1 );

	if ( m_valueStackPtr == 0 )
	{
		// nothing was found
		throw AsmException_SyntaxError_EmptyExpression( m_line, m_column );
	}

	return m_valueStack[ 0 ];
}



/*************************************************************************************************/
/**
	LineParser::EvaluateExpressionAsInt()

	Version of EvaluateExpression which returns its result as an int
*/
/*************************************************************************************************/
int LineParser::EvaluateExpressionAsInt( bool bAllowOneMismatchedCloseBracket )
{
	return static_cast< int >( EvaluateExpression( bAllowOneMismatchedCloseBracket ) );
}



/*************************************************************************************************/
/**
	LineParser::EvalAdd()
*/
/*************************************************************************************************/
void LineParser::EvalAdd()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = m_valueStack[ m_valueStackPtr - 2 ] + m_valueStack[ m_valueStackPtr - 1 ];
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalSubtract()
*/
/*************************************************************************************************/
void LineParser::EvalSubtract()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = m_valueStack[ m_valueStackPtr - 2 ] - m_valueStack[ m_valueStackPtr - 1 ];
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalMultiply()
*/
/*************************************************************************************************/
void LineParser::EvalMultiply()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = m_valueStack[ m_valueStackPtr - 2 ] * m_valueStack[ m_valueStackPtr - 1 ];
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalDivide()
*/
/*************************************************************************************************/
void LineParser::EvalDivide()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	if ( m_valueStack[ m_valueStackPtr - 1 ] == 0.0 )
	{
		throw AsmException_SyntaxError_DivisionByZero( m_line, m_column - 1 );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = m_valueStack[ m_valueStackPtr - 2 ] / m_valueStack[ m_valueStackPtr - 1 ];
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalPower()
*/
/*************************************************************************************************/
void LineParser::EvalPower()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = pow( m_valueStack[ m_valueStackPtr - 2 ], m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;

	if ( errno == ERANGE )
	{
		throw AsmException_SyntaxError_NumberTooBig( m_line, m_column - 1 );
	}

	if ( errno == EDOM )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalDiv()
*/
/*************************************************************************************************/
void LineParser::EvalDiv()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	if ( m_valueStack[ m_valueStackPtr - 1 ] == 0.0 )
	{
		throw AsmException_SyntaxError_DivisionByZero( m_line, m_column - 1 );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] ) /
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalMod()
*/
/*************************************************************************************************/
void LineParser::EvalMod()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	if ( m_valueStack[ m_valueStackPtr - 1 ] == 0.0 )
	{
		throw AsmException_SyntaxError_DivisionByZero( m_line, m_column - 1 );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] ) %
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalShiftLeft()
*/
/*************************************************************************************************/
void LineParser::EvalShiftLeft()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	int val = static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] );
	int shift = static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] );
	int result;

	if ( shift > 31 || shift < -31 )
	{
		result = 0;
	}
	else if ( shift > 0 )
	{
		result = val << shift;
	}
	else if ( shift == 0 )
	{
		result = val;
	}
	else
	{
		result = val >> (-shift);
	}

	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >( result );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalShiftRight()
*/
/*************************************************************************************************/
void LineParser::EvalShiftRight()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	int val = static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] );
	int shift = static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] );
	int result;

	if ( shift > 31 || shift < -31 )
	{
		result = 0;
	}
	else if ( shift > 0 )
	{
		result = val >> shift;
	}
	else if ( shift == 0 )
	{
		result = val;
	}
	else
	{
		result = val << (-shift);
	}

	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >( result );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalAnd()
*/
/*************************************************************************************************/
void LineParser::EvalAnd()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] ) &
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalOr()
*/
/*************************************************************************************************/
void LineParser::EvalOr()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] ) |
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalEor()
*/
/*************************************************************************************************/
void LineParser::EvalEor()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 2 ] ) ^
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalEqual()
*/
/*************************************************************************************************/
void LineParser::EvalEqual()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = -( m_valueStack[ m_valueStackPtr - 2 ] == m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalNotEqual()
*/
/*************************************************************************************************/
void LineParser::EvalNotEqual()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = -( m_valueStack[ m_valueStackPtr - 2 ] != m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalLessThanOrEqual()
*/
/*************************************************************************************************/
void LineParser::EvalLessThanOrEqual()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = -( m_valueStack[ m_valueStackPtr - 2 ] <= m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalMoreThanOrEqual()
*/
/*************************************************************************************************/
void LineParser::EvalMoreThanOrEqual()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = -( m_valueStack[ m_valueStackPtr - 2 ] >= m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalLessThan()
*/
/*************************************************************************************************/
void LineParser::EvalLessThan()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = -( m_valueStack[ m_valueStackPtr - 2 ] < m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalMoreThan()
*/
/*************************************************************************************************/
void LineParser::EvalMoreThan()
{
	if ( m_valueStackPtr < 2 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 2 ] = -( m_valueStack[ m_valueStackPtr - 2 ] > m_valueStack[ m_valueStackPtr - 1 ] );
	m_valueStackPtr--;
}



/*************************************************************************************************/
/**
	LineParser::EvalNegate()
*/
/*************************************************************************************************/
void LineParser::EvalNegate()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = -m_valueStack[ m_valueStackPtr - 1 ];
}



/*************************************************************************************************/
/**
	LineParser::EvalNot()
*/
/*************************************************************************************************/
void LineParser::EvalNot()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = static_cast< double >(
		~static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
}



/*************************************************************************************************/
/**
	LineParser::EvalPosate()
*/
/*************************************************************************************************/
void LineParser::EvalPosate()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	// does absolutely nothing
}



/*************************************************************************************************/
/**
	LineParser::EvalLo()
*/
/*************************************************************************************************/
void LineParser::EvalLo()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) & 0xFF );
}



/*************************************************************************************************/
/**
	LineParser::EvalHi()
*/
/*************************************************************************************************/
void LineParser::EvalHi()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = static_cast< double >(
		( static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) & 0xffff ) >> 8 );
}



/*************************************************************************************************/
/**
	LineParser::EvalSin()
*/
/*************************************************************************************************/
void LineParser::EvalSin()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = sin( m_valueStack[ m_valueStackPtr - 1 ] );
}



/*************************************************************************************************/
/**
	LineParser::EvalCos()
*/
/*************************************************************************************************/
void LineParser::EvalCos()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = cos( m_valueStack[ m_valueStackPtr - 1 ] );
}



/*************************************************************************************************/
/**
	LineParser::EvalTan()
*/
/*************************************************************************************************/
void LineParser::EvalTan()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = tan( m_valueStack[ m_valueStackPtr - 1 ] );
}



/*************************************************************************************************/
/**
	LineParser::EvalArcSin()
*/
/*************************************************************************************************/
void LineParser::EvalArcSin()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = asin( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == EDOM )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalArcCos()
*/
/*************************************************************************************************/
void LineParser::EvalArcCos()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = acos( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == EDOM )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalArcTan()
*/
/*************************************************************************************************/
void LineParser::EvalArcTan()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = atan( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == EDOM )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalLog()
*/
/*************************************************************************************************/
void LineParser::EvalLog()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = log10( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == EDOM || errno == ERANGE )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalLn()
*/
/*************************************************************************************************/
void LineParser::EvalLn()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = log( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == EDOM || errno == ERANGE )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalExp()
*/
/*************************************************************************************************/
void LineParser::EvalExp()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = exp( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == ERANGE )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalSqrt()
*/
/*************************************************************************************************/
void LineParser::EvalSqrt()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = sqrt( m_valueStack[ m_valueStackPtr - 1 ] );

	if ( errno == EDOM )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
}



/*************************************************************************************************/
/**
	LineParser::EvalDegToRad()
*/
/*************************************************************************************************/
void LineParser::EvalDegToRad()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = m_valueStack[ m_valueStackPtr - 1 ] * M_PI / 180.0;
}



/*************************************************************************************************/
/**
	LineParser::EvalRadToDeg()
*/
/*************************************************************************************************/
void LineParser::EvalRadToDeg()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = m_valueStack[ m_valueStackPtr - 1 ] * 180.0 / M_PI;
}



/*************************************************************************************************/
/**
	LineParser::EvalInt()
*/
/*************************************************************************************************/
void LineParser::EvalInt()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = static_cast< double >(
		static_cast< int >( m_valueStack[ m_valueStackPtr - 1 ] ) );
}



/*************************************************************************************************/
/**
	LineParser::EvalAbs()
*/
/*************************************************************************************************/
void LineParser::EvalAbs()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}
	m_valueStack[ m_valueStackPtr - 1 ] = abs( m_valueStack[ m_valueStackPtr - 1 ] );
}



/*************************************************************************************************/
/**
	LineParser::EvalSgn()
*/
/*************************************************************************************************/
void LineParser::EvalSgn()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}

	double val = m_valueStack[ m_valueStackPtr - 1 ];
	m_valueStack[ m_valueStackPtr - 1 ] = ( val < 0.0 ) ? -1.0 : ( ( val > 0.0 ) ? 1.0 : 0.0 );
}



/*************************************************************************************************/
/**
	LineParser::EvalRnd()
*/
/*************************************************************************************************/
void LineParser::EvalRnd()
{
	if ( m_valueStackPtr < 1 )
	{
		throw AsmException_SyntaxError_MissingValue( m_line, m_column );
	}

	double val = m_valueStack[ m_valueStackPtr - 1 ];
	double result = 0.0;

	if ( val < 1.0f )
	{
		throw AsmException_SyntaxError_IllegalOperation( m_line, m_column - 1 );
	}
	else if ( val == 1.0f )
	{
		result = rand() / ( static_cast< double >( RAND_MAX ) + 1.0 );
	}
	else
	{
		result = static_cast< double >( static_cast< int >( rand() / ( static_cast< double >( RAND_MAX ) + 1.0 ) * val ) );
	}

	m_valueStack[ m_valueStackPtr - 1 ] = result;
}
