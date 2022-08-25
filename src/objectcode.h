/*************************************************************************************************/
/**
	objectcode.h


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

#ifndef OBJECTCODE_H_
#define OBJECTCODE_H_

#include <cassert>
#include <cstdlib>


class ObjectCode
{
public:

	static void Create();
	static void Destroy();
	static inline ObjectCode& Instance() { assert( m_gInstance != NULL ); return *m_gInstance; }

	inline void SetPC( int i )		{ m_PC = i; }
	inline int GetPC() const		{ return m_PC; }

	void SetCPU( int i );
	inline int GetCPU() const		{ return m_CPU; }

	inline const unsigned char* GetAddr( int i ) const { return m_aMemory + i; }

	void InitialisePass();

	void PutByte( unsigned int byte );
	void Assemble1( unsigned int opcode );
	void Assemble2( unsigned int opcode, unsigned int val );
	void Assemble3( unsigned int opcode, unsigned int addr );
	void IncBin( const char* filename );

	void SetGuard( int i );
	void Clear( int start, int end, bool bAll = true );

	void SetMapping( int ascii, int mapped );
	int GetMapping( int ascii ) const;

	void CopyBlock( int start, int end, int dest, bool firstPass );

private:

	// Each byte in the memory map has a set of flags
	enum FLAGS
	{
		// This memory location has been used so don't assemble over it
		USED  = (1 << 0),
		// This memory location has been guarded so don't assemble over it
		GUARD = (1 << 1),
		// On the second pass, check that opcodes match what was written on the first pass
		CHECK = (1 << 2),
		// Suppress the opcode check (set by CLEAR)
		DONT_CHECK = (1 << 3)
	};


	ObjectCode();
	~ObjectCode();

	unsigned char				m_aMemory[ 0x10000 ];
	unsigned char				m_aFlags[ 0x10000 ];
	int							m_PC;
	int							m_CPU;

	unsigned char				m_aMapChar[ 96 ];

	static ObjectCode*			m_gInstance;
};



#endif // OBJECTCODE_H_
