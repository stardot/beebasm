/*************************************************************************************************/
/**
	objectcode.h
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

	inline const unsigned char* GetAddr( int i ) const { return m_aMemory + i; }

	void PutByte( unsigned int byte );
	void Assemble1( unsigned int opcode );
	void Assemble2( unsigned int opcode, unsigned int val );
	void Assemble3( unsigned int opcode, unsigned int addr );
	void IncBin( const char* filename );

	void SetGuard( int i );
	void Clear( int start, int end, bool bAll = true );

	void SetMapping( int ascii, int mapped );
	int GetMapping( int ascii ) const;


private:

	enum FLAGS
	{
		USED  = (1 << 0),
		GUARD = (1 << 1),
		CHECK = (1 << 2),
		DONT_CHECK = (1 << 3)
	};


	ObjectCode();
	~ObjectCode();

	unsigned char				m_aMemory[ 0x10000 ];
	unsigned char				m_aFlags[ 0x10000 ];
	int							m_PC;

	unsigned char				m_aMapChar[ 96 ];

	static ObjectCode*			m_gInstance;
};



#endif // OBJECTCODE_H_
