/*************************************************************************************************/
/**
	objectcode.cpp


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

#include <cstring>
#include <iostream>
#include <fstream>

#include "objectcode.h"
#include "symboltable.h"
#include "asmexception.h"
#include "globaldata.h"


ObjectCode* ObjectCode::m_gInstance = NULL;


using namespace std;


/*************************************************************************************************/
/**
	ObjectCode::Create()

	Creates the ObjectCode singleton
*/
/*************************************************************************************************/
void ObjectCode::Create()
{
	assert( m_gInstance == NULL );

	m_gInstance = new ObjectCode;
}



/*************************************************************************************************/
/**
	ObjectCode::Destroy()

	Destroys the ObjectCode singleton
*/
/*************************************************************************************************/
void ObjectCode::Destroy()
{
	assert( m_gInstance != NULL );

	delete m_gInstance;
	m_gInstance = NULL;
}



/*************************************************************************************************/
/**
	ObjectCode::ObjectCode()

	ObjectCode constructor
*/
/*************************************************************************************************/
ObjectCode::ObjectCode()
	:	m_PC( 0 )
{
	memset( m_aMemory, 0, sizeof m_aMemory );
	memset( m_aFlags, 0, sizeof m_aFlags );

	// initialise ascii mapping table

	for ( int i = 0; i < 96; i++ )
	{
		m_aMapChar[ i ] = i + 32;
	}
}



/*************************************************************************************************/
/**
	ObjectCode::~ObjectCode()

	ObjectCode destructor
*/
/*************************************************************************************************/
ObjectCode::~ObjectCode()
{
}



/*************************************************************************************************/
/**
	ObjectCode::PutByte()

	Puts one byte to memory image, never doing pass consistency checks
*/
/*************************************************************************************************/
void ObjectCode::PutByte( unsigned int byte )
{
	if ( m_PC > 0xFFFF )
	{
		throw AsmException_AssembleError_OutOfMemory();
	}

	assert( m_PC >= 0 && m_PC < 0x10000 );
	assert( byte < 0x100 );

	if ( m_aFlags[ m_PC ] & GUARD )
	{
		throw AsmException_AssembleError_GuardHit();
	}

	if ( m_aFlags[ m_PC ] & USED )
	{
		throw AsmException_AssembleError_Overlap();
	}

	m_aFlags[ m_PC ] |= USED;
	m_aMemory[ m_PC++ ] = byte;

	SymbolTable::Instance().ChangeSymbol( "P%", m_PC );
}



/*************************************************************************************************/
/**
	ObjectCode::Assemble1()

	Assembles one byte to memory image
*/
/*************************************************************************************************/
void ObjectCode::Assemble1( unsigned int opcode )
{
	if ( m_PC > 0xFFFF )
	{
		throw AsmException_AssembleError_OutOfMemory();
	}

	assert( m_PC >= 0 && m_PC < 0x10000 );
	assert( opcode < 0x100 );

	if ( GlobalData::Instance().IsSecondPass() &&
		 ( m_aFlags[ m_PC ] & CHECK ) &&
		 !( m_aFlags[ m_PC ] & DONT_CHECK ) &&
		 m_aMemory[ m_PC ] != opcode )
	{
		throw AsmException_AssembleError_InconsistentCode();
	}

	if ( m_aFlags[ m_PC ] & GUARD )
	{
		throw AsmException_AssembleError_GuardHit();
	}

	if ( m_aFlags[ m_PC ] & USED )
	{
		throw AsmException_AssembleError_Overlap();
	}

	m_aFlags[ m_PC ] |= ( USED | CHECK );
	m_aMemory[ m_PC++ ] = opcode;

	SymbolTable::Instance().ChangeSymbol( "P%", m_PC );
}



/*************************************************************************************************/
/**
	ObjectCode::Assemble2()

	Assembles two bytes to memory image
*/
/*************************************************************************************************/
void ObjectCode::Assemble2( unsigned int opcode, unsigned int val )
{
	if ( m_PC > 0xFFFE )
	{
		throw AsmException_AssembleError_OutOfMemory();
	}

	assert( m_PC >= 0 && m_PC < 0x10000 );
	assert( opcode < 0x100 );
	assert( val < 0x100 );

	if ( GlobalData::Instance().IsSecondPass() &&
		 ( m_aFlags[ m_PC ] & CHECK ) &&
		 !( m_aFlags[ m_PC ] & DONT_CHECK ) &&
		 m_aMemory[ m_PC ] != opcode )
	{
		throw AsmException_AssembleError_InconsistentCode();
	}

	if ( ( m_aFlags[ m_PC ] & GUARD ) ||
		 ( m_aFlags[ m_PC + 1 ] & GUARD ) )
	{
		throw AsmException_AssembleError_GuardHit();
	}

	if ( ( m_aFlags[ m_PC ] & USED ) ||
		 ( m_aFlags[ m_PC + 1 ] & USED ) )
	{
		throw AsmException_AssembleError_Overlap();
	}

	m_aFlags[ m_PC ] |= ( USED | CHECK );
	m_aMemory[ m_PC++ ] = opcode;
	m_aFlags[ m_PC ] |= USED;
	m_aMemory[ m_PC++ ] = val;

	SymbolTable::Instance().ChangeSymbol( "P%", m_PC );
}



/*************************************************************************************************/
/**
	ObjectCode::Assemble3()

	Assembles three bytes to memory image
*/
/*************************************************************************************************/
void ObjectCode::Assemble3( unsigned int opcode, unsigned int addr )
{
	if ( m_PC > 0xFFFD )
	{
		throw AsmException_AssembleError_OutOfMemory();
	}

	assert( m_PC >= 0 && m_PC < 0x10000 );
	assert( opcode < 0x100 );
	assert( addr < 0x10000 );

	if ( GlobalData::Instance().IsSecondPass() &&
		 ( m_aFlags[ m_PC ] & CHECK ) &&
		 !( m_aFlags[ m_PC ] & DONT_CHECK ) &&
		 m_aMemory[ m_PC ] != opcode )
	{
		throw AsmException_AssembleError_InconsistentCode();
	}

	if ( ( m_aFlags[ m_PC ] & GUARD ) ||
		 ( m_aFlags[ m_PC + 1 ] & GUARD ) ||
		 ( m_aFlags[ m_PC + 2 ] & GUARD ) )
	{
		throw AsmException_AssembleError_GuardHit();
	}

	if ( ( m_aFlags[ m_PC ] & USED ) ||
		 ( m_aFlags[ m_PC + 1 ] & USED ) ||
		 ( m_aFlags[ m_PC + 2 ] & USED ) )
	{
		throw AsmException_AssembleError_Overlap();
	}

	m_aFlags[ m_PC ] |= ( USED | CHECK );
	m_aMemory[ m_PC++ ] = opcode;
	m_aFlags[ m_PC ] |= USED;
	m_aMemory[ m_PC++ ] = addr & 0xFF;
	m_aFlags[ m_PC ] |= USED;
	m_aMemory[ m_PC++ ] = ( addr & 0xFF00 ) >> 8;

	SymbolTable::Instance().ChangeSymbol( "P%", m_PC );
}



/*************************************************************************************************/
/**
	ObjectCode::SetGuard()
*/
/*************************************************************************************************/
void ObjectCode::SetGuard( int addr )
{
	assert( addr >= 0 && addr < 0x10000 );
	m_aFlags[ addr ] |= GUARD;
}



/*************************************************************************************************/
/**
	ObjectCode::Clear()
*/
/*************************************************************************************************/
void ObjectCode::Clear( int start, int end, bool bAll )
{
	assert( start < end );
	assert( start >= 0 && start < 0x10000 );
	assert( end > 0 && end <= 0x10000 );

	if ( bAll )
	{
		// via CLEAR command
		// as soon as we force a block to be cleared, we can no longer do inconsistency checks on
		// the object code, so we flag the whole block as DONT_CHECK
		memset( m_aMemory + start, 0, end - start );
		memset( m_aFlags + start, DONT_CHECK, end - start );
	}
	else
	{
		// between first and second pass
		// we preserve the memory image and the CHECK flags so that we can test for inconsistencies
		// in the assembled code between first and second passes
		for ( unsigned char* i = m_aFlags + start; i < m_aFlags + end; i++ )
		{
			(*i) &= ( CHECK | DONT_CHECK );
		}
	}
}



/*************************************************************************************************/
/**
	ObjectCode::IncBin()
*/
/*************************************************************************************************/
void ObjectCode::IncBin( const char* filename )
{
	ifstream binfile;

	binfile.open( filename, ios_base::in | ios_base::binary );

	if ( !binfile )
	{
		throw AsmException_AssembleError_FileOpen();
	}

	char c;

	while ( binfile.get( c ) )
	{
		assert( binfile.gcount() == 1 );
		Assemble1( static_cast< unsigned char >( c ) );
	}

	if ( !binfile.eof() )
	{
		throw AsmException_AssembleError_FileRead();
	}

	binfile.close();
}



/*************************************************************************************************/
/**
	ObjectCode::SetMapping()
*/
/*************************************************************************************************/
void ObjectCode::SetMapping( int ascii, int mapped )
{
	assert( ascii > 31 && ascii < 127 );
	assert( mapped >= 0 && mapped < 256 );

	m_aMapChar[ ascii - 32 ] = mapped;
}



/*************************************************************************************************/
/**
	ObjectCode::SetMapping()
*/
/*************************************************************************************************/
int ObjectCode::GetMapping( int ascii ) const
{
	assert( ascii > 31 && ascii < 127 );
	return m_aMapChar[ ascii - 32 ];
}
