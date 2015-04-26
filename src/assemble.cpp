/*************************************************************************************************/
/**
	assemble.cpp

	Contains all the LineParser methods for assembling code


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
#include <cstring>

#include "lineparser.h"
#include "globaldata.h"
#include "objectcode.h"
#include "asmexception.h"


using namespace std;



#define DATA( cpu, op, imp, acc, imm, zp, zpx, zpy, abs, absx, absy, ind, indx, indy, ind16, ind16x, rel )  \
	{ { imp, acc, imm, zp, zpx, zpy, abs, absx, absy, ind, indx, indy, ind16, ind16x, rel }, op, cpu }

#define X -1

const LineParser::OpcodeData	LineParser::m_gaOpcodeTable[] =
{
//					IMP		ACC		IMM		ZP		ZPX		ZPY		ABS		ABSX	ABSY	IND		INDX	INDY	IND16	IND16X	REL

	DATA( 0, "ADC",	 X,		 X,		0x69,	0x65,	0x75,	 X,		0x6D,	0x7D,	0x79,	0x172,	0x61,	0x71,	 X,		 X,		 X		),
	DATA( 0, "AND",	 X,		 X,		0x29,	0x25,	0x35,	 X,		0x2D,	0x3D,	0x39,	0x132,	0x21,	0x31,	 X,		 X,		 X		),
	DATA( 0, "ASL",	 X,		0x0A,	 X,		0x06,	0x16,	 X,		0x0E,	0x1E,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "BCC",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x90	),
	DATA( 0, "BCS",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0xB0	),
	DATA( 0, "BEQ",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0xF0	),
	DATA( 0, "BIT",	 X,		 X,		0x189,	0x24,	0x134,	 X,		0x2C,	0x13C,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "BMI",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x30	),
	DATA( 0, "BNE",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0xD0	),
	DATA( 0, "BPL",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x10	),
	DATA( 1, "BRA",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x180	),
	DATA( 0, "BRK",	0x00,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "BVC",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x50	),
	DATA( 0, "BVS",	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		0x70	),
	DATA( 0, "CLC",	0x18,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "CLD",	0xD8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "CLI",	0x58,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "CLR",	 X,		 X,		 X,		0x164,	0x174,	 X,		0x19C,	0x19E,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "CLV",	0xB8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "CMP",	 X,		 X,		0xC9,	0xC5,	0xD5,	 X,		0xCD,	0xDD,	0xD9,	0x1D2,	0xC1,	0xD1,	 X,		 X,		 X		),
	DATA( 0, "CPX",	 X,		 X,		0xE0,	0xE4,	 X,		 X,		0xEC,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "CPY",	 X,		 X,		0xC0,	0xC4,	 X,		 X,		0xCC,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "DEA",	0x13A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "DEC",	 X,		0x13A,	 X,		0xC6,	0xD6,	 X,		0xCE,	0xDE,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "DEX",	0xCA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "DEY",	0x88,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "EOR",	 X,		 X,		0x49,	0x45,	0x55,	 X,		0x4D,	0x5D,	0x59,	0x152,	0x41,	0x51,	 X,		 X,		 X		),
	DATA( 1, "INA",	0x11A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "INC",	 X,		0x11A,	 X,		0xE6,	0xF6,	 X,		0xEE,	0xFE,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "INX",	0xE8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "INY",	0xC8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "JMP",	 X,		 X,		 X,		 X,		 X,		 X,		0x4C,	 X,		 X,		 X,		 X,		 X,		0x6C,	0x17C,	 X		),
	DATA( 0, "JSR",	 X,		 X,		 X,		 X,		 X,		 X,		0x20,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "LDA",	 X,		 X,		0xA9,	0xA5,	0xB5,	 X,		0xAD,	0xBD,	0xB9,	0x1B2,	0xA1,	0xB1,	 X,		 X,		 X		),
	DATA( 0, "LDX",	 X,		 X,		0xA2,	0xA6,	 X,		0xB6,	0xAE,	 X,		0xBE,	 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "LDY",	 X,		 X,		0xA0,	0xA4,	0xB4,	 X,		0xAC,	0xBC,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "LSR",	 X,		0x4A,	 X,		0x46,	0x56,	 X,		0x4E,	0x5E,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "NOP",	0xEA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "ORA",	 X,		 X,		0x09,	0x05,	0x15,	 X,		0x0D,	0x1D,	0x19,	0x112,	0x01,	0x11,	 X,		 X,		 X		),
	DATA( 0, "PHA",	0x48,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "PHP",	0x08,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "PHX",	0x1DA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "PHY",	0x15A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "PLA",	0x68,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "PLP",	0x28,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "PLX",	0x1FA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "PLY",	0x17A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "ROL",	 X,		0x2A,	 X,		0x26,	0x36,	 X,		0x2E,	0x3E,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "ROR",	 X,		0x6A,	 X,		0x66,	0x76,	 X,		0x6E,	0x7E,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "RTI",	0x40,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "RTS",	0x60,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "SBC",	 X,		 X,		0xE9,	0xE5,	0xF5,	 X,		0xED,	0xFD,	0xF9,	0x1F2,	0xE1,	0xF1,	 X,		 X,		 X		),
	DATA( 0, "SEC",	0x38,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "SED",	0xF8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "SEI",	0x78,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "STA",	 X,		 X,		 X,		0x85,	0x95,	 X,		0x8D,	0x9D,	0x99,	0x192,	0x81,	0x91,	 X,		 X,		 X		),
	DATA( 0, "STX",	 X,		 X,		 X,		0x86,	 X,		0x96,	0x8E,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "STY",	 X,		 X,		 X,		0x84,	0x94,	 X,		0x8C,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "STZ",	 X,		 X,		 X,		0x164,	0x174,	 X,		0x19C,	0x19E,	 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "TAX",	0xAA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "TAY",	0xA8,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "TRB",	 X,		 X,		 X,		0x114,	 X,		 X,		0x11C,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 1, "TSB",	 X,		 X,		 X,		0x104,	 X,		 X,		0x10C,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "TSX",	0xBA,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "TXA",	0x8A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "TXS",	0x9A,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		),
	DATA( 0, "TYA",	0x98,	 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X,		 X		)
};

#undef X


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
		int			cpu		= m_gaOpcodeTable[ i ].m_cpu;
		const char*	token	= m_gaOpcodeTable[ i ].m_pName;
		size_t		len		= strlen( token );

		// ignore instructions not for current cpu
		if ( cpu > ObjectCode::Instance().GetCPU() )
			continue;

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
	LineParser::HasAddressingMode()
*/
/*************************************************************************************************/
bool LineParser::HasAddressingMode( int instructionIndex, ADDRESSING_MODE mode )
{
	int i = m_gaOpcodeTable[ instructionIndex ].m_aOpcodes[ mode ];
	return ( i != -1 && (i & 0xFF00) <= (ObjectCode::Instance().GetCPU() << 8) );
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
	return static_cast< unsigned int >( i & 0xFF );
}



/*************************************************************************************************/
/**
	LineParser::Assemble1()
*/
/*************************************************************************************************/
void LineParser::Assemble1( int instructionIndex, ADDRESSING_MODE mode )
{
	assert( HasAddressingMode( instructionIndex, mode ) );

	if ( ostream *o = GlobalData::Instance().GetVerboseAsmOutputStream() )
	{
		*o << uppercase << hex << setfill( '0' ) << "     ";
		*o << setw(4) << ObjectCode::Instance().GetPC() << "   ";
		*o << setw(2) << GetOpcode( instructionIndex, mode ) << "         ";
		*o << m_gaOpcodeTable[ instructionIndex ].m_pName;

		if ( mode == ACC )
		{
			*o << " A";
		}

		*o << endl << nouppercase << dec << setfill( ' ' );
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

	if ( ostream *o = GlobalData::Instance().GetVerboseAsmOutputStream() )
	{
		*o << uppercase << hex << setfill( '0' ) << "     ";
		*o << setw(4) << ObjectCode::Instance().GetPC() << "   ";
		*o << setw(2) << GetOpcode( instructionIndex, mode ) << " ";
		*o << setw(2) << value << "      ";
		*o << m_gaOpcodeTable[ instructionIndex ].m_pName << " ";

		if ( mode == IMM )
		{
			*o << "#";
		}
		else if ( mode == IND || mode == INDX || mode == INDY )
		{
			*o << "(";
		}

		if ( mode == REL )
		{
			*o << "&" << setw(4) << ObjectCode::Instance().GetPC() + 2 + static_cast< signed char >( value );
		}
		else
		{
			*o << "&" << setw(2) << value;
		}

		if ( mode == ZPX )
		{
			*o << ",X";
		}
		else if ( mode == ZPY )
		{
			*o << ",Y";
		}
		else if ( mode == IND )
		{
			*o << ")";
		}
		else if ( mode == INDX )
		{
			*o << ",X)";
		}
		else if ( mode == INDY )
		{
			*o << "),Y";
		}

		*o << endl << nouppercase << dec << setfill( ' ' );
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

	if ( ostream *o = GlobalData::Instance().GetVerboseAsmOutputStream() )
	{
		*o << uppercase << hex << setfill( '0' ) << "     ";
		*o << setw(4) << ObjectCode::Instance().GetPC() << "   ";
		*o << setw(2) << GetOpcode( instructionIndex, mode ) << " ";
		*o << setw(2) << ( value & 0xFF ) << " ";
		*o << setw(2) << ( ( value >> 8 ) & 0xFF ) << "   ";
		*o << m_gaOpcodeTable[ instructionIndex ].m_pName << " ";

		if ( mode == IND16 || mode == IND16X )
		{
			*o << "(";
		}

		*o << "&" << setw(4) << value;

		if ( mode == ABSX )
		{
			*o << ",X";
		}
		else if ( mode == ABSY )
		{
			*o << ",Y";
		}
		else if ( mode == IND16 )
		{
			*o << ")";
		}
		else if ( mode == IND16X )
		{
			*o << ",X)";
		}

		*o << endl << nouppercase << dec << setfill( ' ' );
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

	if ( m_column < m_line.length() && m_line[ m_column ] == '#' )
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
			// Immediate constant too large
			throw AsmException_SyntaxError_ImmTooLarge( m_line, oldColumn );
		}

		if ( value < 0 )
		{
			// Immediate constant is negative
			throw AsmException_SyntaxError_ImmNegative( m_line, oldColumn );
		}

		if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
		{
			// Unexpected comma (remembering that an expression can validly end with a comma)
			throw AsmException_SyntaxError_UnexpectedComma( m_line, m_column );
		}

		// Actually assemble the instruction
		Assemble2( instruction, IMM, static_cast< unsigned int >( value ) );
		return;
	}

	// see if it's accumulator mode

	if ( m_column < m_line.length() && toupper( m_line[ m_column ] ) == 'A' && HasAddressingMode( instruction, ACC ) )
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

	if ( m_column < m_line.length() && m_line[ m_column ] == '(' )
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

		// the only valid character to find here is ',' for (ind,X) or (ind16,X) and ')' for (ind),Y or (ind16) or (ind)
		// we know that ind and ind16 forms are exclusive
		// check (ind), (ind16) and (ind),Y

		if ( m_column < m_line.length() && m_line[ m_column ] == ')' )
		{
			m_column++;

			// check (ind) and (ind16)

			if ( !AdvanceAndCheckEndOfStatement() )
			{
				// nothing else here - must be ind or ind16... see if this is allowed!

				if ( HasAddressingMode( instruction, IND16 ) )
				{
					// It is definitely ind16 mode - check for the 6502 bug

					if ( ( value & 0xFF ) == 0xFF )
					{
						// victim of the 6502 bug!  throw an error
						throw AsmException_SyntaxError_6502Bug( m_line, oldColumn + 1 );
					}

					Assemble3( instruction, IND16, value );
					return;
				}

				if ( !HasAddressingMode( instruction, IND ) )
				{
					throw AsmException_SyntaxError_NoIndirect( m_line, oldColumn );
				}

				// assemble (ind) instruction

				if ( value > 0xFF )
				{
					// it's not ZP and it must be
					throw AsmException_SyntaxError_NotZeroPage( m_line, oldColumn + 1 );
				}

				if ( value < 0 )
				{
					throw AsmException_SyntaxError_BadAddress( m_line, oldColumn + 1 );
				}

				Assemble2( instruction, IND, value );
				return;
			}

			// if we find ,Y then it's an (ind),Y

			if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
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

				if ( value < 0 )
				{
					throw AsmException_SyntaxError_BadAddress( m_line, oldColumn + 1 );
				}

				Assemble2( instruction, INDY, value );
				return;
			}

			// If we got here, we identified neither (ind16) nor (ind),Y
			// Therefore we throw a syntax error

			throw AsmException_SyntaxError_BadIndirect( m_line, m_column );
		}

		// check (ind,X) or (ind16,X)

		if ( m_column < m_line.length() && m_line[ m_column ] == ',' )
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

			if ( HasAddressingMode( instruction, IND16X ) )
			{
				// It is definitely ind16,x mode

				Assemble3( instruction, IND16X, value );
				return;
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

			if ( value < 0 )
			{
				throw AsmException_SyntaxError_BadAddress( m_line, oldColumn + 1 );
			}

			Assemble2( instruction, INDX, value );
			return;
		}

		// If we got here, we identified none of (ind), (ind16), (ind,X), (ind16,X) or (ind),Y
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
	catch ( AsmException_SyntaxError_SymbolNotDefined& )
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

		if ( value < 0 || value > 0xFFFF )
		{
			throw AsmException_SyntaxError_BadAddress( m_line, oldColumn );
		}

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

	if ( m_column >= m_line.length() || m_line[ m_column ] != ',' )
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

	if ( m_column < m_line.length() && toupper( m_line[ m_column ] ) == 'X' )
	{
		m_column++;

		if ( AdvanceAndCheckEndOfStatement() )
		{
			// We were not expecting any more characters
			throw AsmException_SyntaxError_BadIndexed( m_line, m_column );
		}

		if ( value < 0 || value > 0xFFFF )
		{
			throw AsmException_SyntaxError_BadAddress( m_line, oldColumn );
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

	if ( m_column < m_line.length() && toupper( m_line[ m_column ] ) == 'Y' )
	{
		m_column++;

		if ( AdvanceAndCheckEndOfStatement() )
		{
			// We were not expecting any more characters
			throw AsmException_SyntaxError_BadIndexed( m_line, m_column );
		}

		if ( value < 0 || value > 0xFFFF )
		{
			throw AsmException_SyntaxError_BadAddress( m_line, oldColumn );
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
