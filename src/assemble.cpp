/*************************************************************************************************/
/**
	assemble.cpp

	Contains all the LineParser methods for assembling code
*/
/*************************************************************************************************/

#include <iostream>
#include <iomanip>

#include "lineparser.h"
#include "globaldata.h"
#include "objectcode.h"
#include "asmexception.h"


using namespace std;



#define DATA( op, imp, acc, imm, zp, zpx, zpy, abs, absx, absy, indx, indy, ind16, rel )  \
	{ { imp, acc, imm, zp, zpx, zpy, abs, absx, absy, indx, indy, ind16, rel }, op }

#define X -1

LineParser::OpcodeData	LineParser::m_gaOpcodeTable[] =
{
//					IMP		ACC		IMM		ZP		ZPX		ZPY		ABS		ABSX	ABSY	INDX	INDY	IND16	REL

	DATA( "ADC",	 X,		 X,		0x69,	0x65,	0x75,	 X,		0x6D,	0x7D,	0x79,	0x61,	0x71,	 X,		 X		),
	DATA( "AND",	 X,		 X,		0x29,	0x25,	0x35,	 X,		0x2D,	0x3D,	0x39,	0x21,	0x31,	 X,		 X		),
	DATA( "ASL",	 X,		0x0A,	 X,		0x06,	0x16,	 X,		0x0E,	0x1E,	 X,		 X,		 X,		 X,		 X		),
	DATA( "BCC",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x90	),
	DATA( "BCS",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0xB0	),
	DATA( "BEQ",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0xF0	),
	DATA( "BIT",	 X,		 X,		 X,		0x24,	 X,		 X,		0x2C,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "BMI",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x30	),
	DATA( "BNE",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0xD0	),
	DATA( "BPL",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x10	),
	DATA( "BRK",	0x00,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "BVC",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x50	),
	DATA( "BVS",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x70	),
	DATA( "CLC",	0x18,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "CLD",	0xD8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "CLI",	0x58,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "CLV",	0xB8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "CMP",	 X,		 X,		0xC9,	0xC5,	0xD5,	 X,		0xCD,	0xDD,	0xD9,	0xC1,	0xD1,	 X,		 X		),
	DATA( "CPX",	 X,		 X,		0xE0,	0xE4,	 X,		 X,		0xEC,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "CPY",	 X,		 X,		0xC0,	0xC4,	 X,		 X,		0xCC,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "DEC",	 X,		 X,		 X,		0xC6,	0xD6,	 X,		0xCE,	0xDE,	 X,		 X,		 X,		 X,		 X		),
	DATA( "DEX",	0xCA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "DEY",	0x88,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "EOR",	 X,		 X,		0x49,	0x45,	0x55,	 X,		0x4D,	0x5D,	0x59,	0x41,	0x51,	 X,		 X		),
	DATA( "INC",	 X,		 X,		 X,		0xE6,	0xF6,	 X,		0xEE,	0xFE,	 X,		 X,		 X,		 X,		 X		),
	DATA( "INX",	0xE8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "INY",	0xC8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "JMP",	 X,		 X,		 X,		 X,		 X,		 X,		0x4C,	 X,		 X,		 X,		 X,		0x6C,	 X		),
	DATA( "JSR",	 X,		 X,		 X,		 X,		 X,		 X,		0x20,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "LDA",	 X,		 X,		0xA9,	0xA5,	0xB5,	 X,		0xAD,	0xBD,	0xB9,	0xA1,	0xB1,	 X,		 X		),
	DATA( "LDX",	 X,		 X,		0xA2,	0xA6,	 X,		0xB6,	0xAE,	 X,		0xBE,	 X,		 X,		 X,		 X		),
	DATA( "LDY",	 X,		 X,		0xA0,	0xA4,	0xB4,	 X,		0xAC,	0xBC,	 X,		 X,		 X,		 X,		 X		),
	DATA( "LSR",	 X,		0x4A,	 X,		0x46,	0x56,	 X,		0x4E,	0x5E,	 X,		 X,		 X,		 X,		 X		),
	DATA( "NOP",	0xEA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "ORA",	 X,		 X,		0x09,	0x05,	0x15,	 X,		0x0D,	0x1D,	0x19,	0x01,	0x11,	 X,		 X		),
	DATA( "PHA",	0x48,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "PHP",	0x08,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "PLA",	0x68,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "PLP",	0x28,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "ROL",	 X,		0x2A,	 X,		0x26,	0x36,	 X,		0x2E,	0x3E,	 X,		 X,		 X,		 X,		 X		),
	DATA( "ROR",	 X,		0x6A,	 X,		0x66,	0x76,	 X,		0x6E,	0x7E,	 X,		 X,		 X,		 X,		 X		),
	DATA( "RTI",	0x40,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "RTS",	0x60,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "SBC",	 X,		 X,		0xE9,	0xE5,	0xF5,	 X,		0xED,	0xFD,	0xF9,	0xE1,	0xF1,	 X,		 X		),
	DATA( "SEC",	0x38,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "SED",	0xF8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "SEI",	0x78,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "STA",	 X,		 X,		 X,		0x85,	0x95,	 X,		0x8D,	0x9D,	0x99,	0x81,	0x91,	 X,		 X		),
	DATA( "STX",	 X,		 X,		 X,		0x86,	 X,		0x96,	0x8E,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "STY",	 X,		 X,		 X,		0x84,	0x94,	 X,		0x8C,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "TAX",	0xAA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "TAY",	0xA8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "TSX",	0xBA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "TXA",	0x8A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "TXS",	0x9A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( "TYA",	0x98,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		)
};



/*************************************************************************************************/
/**
	LineParser::GetInstructionAndAdvanceColumn()

	Searches for an instruction match in the current line, starting at the current column,
	and moves the column pointer past the token

	@param		line			The string to parse
	@param		column			The column to start from

	@return		The token number, or -1 for "not found"
				column is modified to index the character after the token
*/
/*************************************************************************************************/
int LineParser::GetInstructionAndAdvanceColumn()
{
	for ( int i = 0; i < static_cast<int>( sizeof m_gaOpcodeTable / sizeof( OpcodeData ) ); i++ )
	{
		const char*	token	= m_gaOpcodeTable[ i ].m_pName;
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
	LineParser::HasAddressingMode()
*/
/*************************************************************************************************/
bool LineParser::HasAddressingMode( int instructionIndex, ADDRESSING_MODE mode )
{
	return ( m_gaOpcodeTable[ instructionIndex ].m_aOpcodes[ mode ] != -1 );
}



/*************************************************************************************************/
/**
	LineParser::GetOpcode()
*/
/*************************************************************************************************/
unsigned int LineParser::GetOpcode( int instructionIndex, ADDRESSING_MODE mode )
{
	int i = m_gaOpcodeTable[ instructionIndex ].m_aOpcodes[ mode ];

	assert( i != -1 );
	return static_cast< unsigned int >( i );
}



/*************************************************************************************************/
/**
	LineParser::Assemble1()
*/
/*************************************************************************************************/
void LineParser::Assemble1( int instructionIndex, ADDRESSING_MODE mode )
{
	assert( HasAddressingMode( instructionIndex, mode ) );

	if ( GlobalData::Instance().ShouldOutputAsm() )
	{
		cout << uppercase << hex << setfill( '0' ) << "     ";
		cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
		cout << setw(2) << GetOpcode( instructionIndex, mode ) << "         ";
		cout << m_gaOpcodeTable[ instructionIndex ].m_pName;

		if ( mode == ACC )
		{
			cout << " A";
		}

		cout << endl << nouppercase << dec << setfill( ' ' );
	}

	try
	{
		ObjectCode::Instance().Assemble1( GetOpcode( instructionIndex, mode ) );
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
	LineParser::Assemble2()
*/
/*************************************************************************************************/
void LineParser::Assemble2( int instructionIndex, ADDRESSING_MODE mode, unsigned int value )
{
	assert( value < 0x100 );
	assert( HasAddressingMode( instructionIndex, mode ) );

	if ( GlobalData::Instance().ShouldOutputAsm() )
	{
		cout << uppercase << hex << setfill( '0' ) << "     ";
		cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
		cout << setw(2) << GetOpcode( instructionIndex, mode ) << " ";
		cout << setw(2) << value << "      ";
		cout << m_gaOpcodeTable[ instructionIndex ].m_pName << " ";

		if ( mode == IMM )
		{
			cout << "#";
		}
		else if ( mode == INDX || mode == INDY )
		{
			cout << "(";
		}

		if ( mode == REL )
		{
			cout << "&" << setw(4) << ObjectCode::Instance().GetPC() + 2 + static_cast< signed char >( value );
		}
		else
		{
			cout << "&" << setw(2) << value;
		}

		if ( mode == ZPX )
		{
			cout << ",X";
		}
		else if ( mode == ZPY )
		{
			cout << ",Y";
		}
		else if ( mode == INDX )
		{
			cout << ",X)";
		}
		else if ( mode == INDY )
		{
			cout << "),Y";
		}

		cout << endl << nouppercase << dec << setfill( ' ' );
	}

	try
	{
		ObjectCode::Instance().Assemble2( GetOpcode( instructionIndex, mode ), value );
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
	LineParser::Assemble3()
*/
/*************************************************************************************************/
void LineParser::Assemble3( int instructionIndex, ADDRESSING_MODE mode, unsigned int value )
{
	assert( value < 0x10000 );
	assert( HasAddressingMode( instructionIndex, mode ) );

	if ( GlobalData::Instance().ShouldOutputAsm() )
	{
		cout << uppercase << hex << setfill( '0' ) << "     ";
		cout << setw(4) << ObjectCode::Instance().GetPC() << "   ";
		cout << setw(2) << GetOpcode( instructionIndex, mode ) << " ";
		cout << setw(2) << ( value & 0xFF ) << " ";
		cout << setw(2) << ( ( value >> 8 ) & 0xFF ) << "   ";
		cout << m_gaOpcodeTable[ instructionIndex ].m_pName << " ";

		if ( mode == IND16 )
		{
			cout << "(";
		}

		cout << "&" << setw(4) << value;

		if ( mode == ABSX )
		{
			cout << ",X";
		}
		else if ( mode == ABSY )
		{
			cout << ",Y";
		}
		else if ( mode == IND16 )
		{
			cout << ")";
		}

		cout << endl << nouppercase << dec << setfill( ' ' );
	}

	try
	{
		ObjectCode::Instance().Assemble3( GetOpcode( instructionIndex, mode ), value );
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
	LineParser::HandleAssembler()
*/
/*************************************************************************************************/
void LineParser::HandleAssembler( int instruction )
{
	int oldColumn = m_column;

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// there is nothing following the opcode - maybe implied mode... see if this is allowed!

		if ( HasAddressingMode( instruction, IMP ) )
		{
			// It's allowed - assemble this instruction
			Assemble1( instruction, IMP );
			return;
		}
		else
		{
			// Implied addressing mode not allowed
			throw AsmException_SyntaxError_NoImplied( m_line, oldColumn );
		}
	}

	// OK, something follows... maybe it's immediate mode

	if ( m_line[ m_column ] == '#' )
	{
		if ( !HasAddressingMode( instruction, IMM ) )
		{
			// Immediate addressing mode not allowed
			throw AsmException_SyntaxError_NoImmediate( m_line, m_column );
		}

		m_column++;
		oldColumn = m_column;

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
			// Immediate constant too large
			throw AsmException_SyntaxError_ImmTooLarge( m_line, oldColumn );
		}

		if ( value < 0 )
		{
			// Immediate constant is negative
			throw AsmException_SyntaxError_ImmNegative( m_line, oldColumn );
		}

		if ( m_line[ m_column ] == ',' )
		{
			// Unexpected comma (remembering that an expression can validly end with a comma)
			throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
		}

		// Actually assemble the instruction
		Assemble2( instruction, IMM, static_cast< unsigned int >( value ) );
		return;
	}

	// see if it's accumulator mode

	if ( toupper( m_line[ m_column ] ) == 'A' && HasAddressingMode( instruction, ACC ) )
	{
		// might be... but only if the next character is a separator or whitespace
		// otherwise, we must assume a label beginning with A

		int rememberColumn = m_column;

		m_column++;

		if ( !AdvanceAndCheckEndOfStatement() )
		{
			// It is definitely accumulator mode - assemble this instruction
			Assemble1( instruction, ACC );
			return;
		}
		else
		{
			// No - restore pointer so we can consider 'A' as the start of a label name later
			m_column = rememberColumn;
		}
	}

	// see if it's (ind,X), (ind),Y or (ind16)

	if ( m_line[ m_column ] == '(' )
	{
		oldColumn = m_column;
		m_column++;

		int value;

		try
		{
			// passing true to EvaluateExpression is a hack which allows us to terminate the expression by
			// an extra close bracket.
			value = EvaluateExpressionAsInt( true );
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

		// the only valid character to find here is ',' for (ind,X) and ')' for (ind),Y and (ind16)

		// check (ind16) and (ind),Y

		if ( m_line[ m_column ] == ')' )
		{
			m_column++;

			// check (ind16)

			if ( !AdvanceAndCheckEndOfStatement() )
			{
				// nothing else here - must be ind16... see if this is allowed!

				if ( !HasAddressingMode( instruction, IND16 ) )
				{
					throw AsmException_SyntaxError_NoIndirect16( m_line, oldColumn );
				}

				// It is definitely ind16 mode - check for the 6502 bug

				if ( ( value & 0xFF ) == 0xFF )
				{
					// victim of the 6502 bug!  throw an error
					throw AsmException_SyntaxError_6502Bug( m_line, oldColumn + 1 );
				}

				Assemble3( instruction, IND16, value );
				return;
			}

			// if we find ,Y then it's an (ind),Y

			if ( m_line[ m_column ] == ',' )
			{
				m_column++;

				if ( !AdvanceAndCheckEndOfStatement() )
				{
					// We expected more characters but there were none
					throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
				}

				if ( toupper( m_line[ m_column ] ) != 'Y' )
				{
					// We were expecting an Y
					throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
				}

				m_column++;

				if ( AdvanceAndCheckEndOfStatement() )
				{
					// We were not expecting any more characters
					throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
				}

				// It is definitely (ind),Y - check we can use it

				if ( !HasAddressingMode( instruction, INDY ) )
				{
					// addressing mode not allowed
					throw AsmException_SyntaxError_NoIndirect( m_line, oldColumn );
				}

				// assemble (ind),Y instruction

				if ( value > 0xFF )
				{
					// it's not ZP and it must be
					throw AsmException_SyntaxError_NotZeroPage( m_line, oldColumn + 1 );
				}

				Assemble2( instruction, INDY, value );
				return;
			}

			// If we got here, we identified neither (ind16) nor (ind),Y
			// Therefore we throw a syntax error

			throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
		}

		// check (ind,X)

		if ( m_line[ m_column ] == ',' )
		{
			m_column++;

			if ( !AdvanceAndCheckEndOfStatement() )
			{
				// We expected more characters but there were none
				throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
			}

			if ( toupper( m_line[ m_column ] ) != 'X' )
			{
				// We were expecting an X
				throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
			}

			m_column++;

			if ( !AdvanceAndCheckEndOfStatement() )
			{
				// We expected more characters but there were none
				throw AsmException_SyntaxError_MismatchedParentheses( m_line, m_column );
			}

			if ( m_line[ m_column ] != ')' )
			{
				// We were expecting a close bracket
				throw AsmException_SyntaxError_MismatchedParentheses( m_line, m_column );
			}

			m_column++;

			if ( AdvanceAndCheckEndOfStatement() )
			{
				// We were not expecting any more characters
				throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
			}

			// It is definitely (ind,X) - check we can use it

			if ( !HasAddressingMode( instruction, INDX ) )
			{
				// addressing mode not allowed
				throw AsmException_SyntaxError_NoIndirect( m_line, oldColumn );
			}

			// It is definitely (ind,X) - assemble this instruction

			if ( value > 0xFF )
			{
				// it's not ZP and it must be
				throw AsmException_SyntaxError_NotZeroPage( m_line, oldColumn + 1 );
			}

			Assemble2( instruction, INDX, value );
			return;
		}

		// If we got here, we identified neither (ind16), (ind,X) nor (ind),Y
		// Therefore we throw a syntax error

		throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
	}

	// if we got here, it must be abs abs,X abs,Y zp zp,X or zp,Y
	// we give priority to trying to match zp as they are the preference

	// get the address operand

	oldColumn = m_column;
	int value;

	try
	{
		value = EvaluateExpressionAsInt();
	}
	catch ( AsmException_SyntaxError_SymbolNotDefined& e )
	{
		if ( GlobalData::Instance().IsFirstPass() )
		{
			// this allows branches to assemble when the value is unknown due to a label not having
			// yet been defined.  Also, this is most likely a 16-bit value, which is a sensible
			// default addressing mode to assume.
			value = ObjectCode::Instance().GetPC();
		}
		else
		{
			throw;
		}
	}

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// end of this instruction

		// see if this is relative addressing (branch)

		if ( HasAddressingMode( instruction, REL ) )
		{
			int branchAmount = value - ( ObjectCode::Instance().GetPC() + 2 );

			if ( branchAmount >= -128 && branchAmount <= 127 )
			{
				Assemble2( instruction, REL, branchAmount & 0xFF );
				return;
			}
			else
			{
				throw AsmException_SyntaxError_BranchOutOfRange( m_line, oldColumn );
			}
		}

		// else this must be abs or zp
		// we assemble abs or zp depending on whether 'value' is a 16- or 8-bit number.
		// we contrive that unknown labels will get a 16-bit value so that absolute addressing is the default.

		if ( value < 0x100 && HasAddressingMode( instruction, ZP ) )
		{
			Assemble2( instruction, ZP, value );
			return;
		}
		else if ( HasAddressingMode( instruction, ABS ) )
		{
			Assemble3( instruction, ABS, value );
			return;
		}
		else
		{
			throw AsmException_SyntaxError_NoAbsolute( m_line, oldColumn );
		}
	}

	// finally, check for indexed versions of the opcode

	if ( m_line[ m_column ] != ',' )
	{
		// weird character - throw error
		throw AsmException_SyntaxError_BadAbsolute( m_line, m_column );
	}

	m_column++;

	if ( !AdvanceAndCheckEndOfStatement() )
	{
		// We expected more characters but there were none
		throw AsmException_SyntaxError_BadAbsolute( m_line, m_column );
	}

	if ( toupper( m_line[ m_column ] == 'X' ) )
	{
		m_column++;

		if ( AdvanceAndCheckEndOfStatement() )
		{
			// We were not expecting any more characters
			throw AsmException_SyntaxError_BadIndexed( m_line, m_column );
		}

		if ( value < 0x100 && HasAddressingMode( instruction, ZPX ) )
		{
			Assemble2( instruction, ZPX, value );
			return;
		}
		else if ( HasAddressingMode( instruction, ABSX ) )
		{
			Assemble3( instruction, ABSX, value );
			return;
		}
		else
		{
			throw AsmException_SyntaxError_NoIndexedX( m_line, oldColumn );
		}
	}

	if ( toupper( m_line[ m_column ] == 'Y' ) )
	{
		m_column++;

		if ( AdvanceAndCheckEndOfStatement() )
		{
			// We were not expecting any more characters
			throw AsmException_SyntaxError_BadIndexed( m_line, m_column );
		}

		if ( value < 0x100 && HasAddressingMode( instruction, ZPY ) )
		{
			Assemble2( instruction, ZPY, value );
			return;
		}
		else if ( HasAddressingMode( instruction, ABSY ) )
		{
			Assemble3( instruction, ABSY, value );
			return;
		}
		else
		{
			throw AsmException_SyntaxError_NoIndexedY( m_line, oldColumn );
		}
	}

	// If we got here, we received a weird index, like LDA addr,Z

	throw AsmException_SyntaxError_BadIndexed( m_line, m_column );
}
