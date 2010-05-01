/*************************************************************************************************/
/**
	globaldata.h
*/
/*************************************************************************************************/

#ifndef GLOBALDATA_H_
#define GLOBALDATA_H_

#include <cassert>
#include <cstdlib>


class DiscImage;

class GlobalData
{
public:

	static void Create();
	static void Destroy();
	static inline GlobalData& Instance() { assert( m_gInstance != NULL ); return *m_gInstance; }

	inline void SetPass( int i )				{ m_pass = i; }
	inline void SetBootFile( const char* p )	{ m_pBootFile = p; }
	inline void SetVerbose( bool b )			{ m_bVerbose = b; }
	inline void SetUseDiscImage( bool b )		{ m_bUseDiscImage = b; }
	inline void SetDiscImage( DiscImage* d )	{ m_pDiscImage = d; }
	inline void ResetForId()					{ m_forId = 0; }

	inline int GetPass() const					{ return m_pass; }
	inline bool IsFirstPass() const				{ return ( m_pass == 0 ); }
	inline bool IsSecondPass() const			{ return ( m_pass == 1 ); }
	inline bool ShouldOutputAsm() const			{ return ( m_pass == 1 && m_bVerbose ); }
	inline const char* GetBootFile() const		{ return m_pBootFile; }
	inline bool UsesDiscImage() const			{ return m_bUseDiscImage; }
	inline DiscImage* GetDiscImage() const		{ return m_pDiscImage; }
	inline int GetNextForId()					{ return m_forId++; }


private:

	GlobalData();
	~GlobalData();

	static GlobalData*			m_gInstance;

	int							m_pass;
	const char*					m_pBootFile;
	bool						m_bVerbose;
	bool						m_bUseDiscImage;
	DiscImage*					m_pDiscImage;
	int							m_forId;
	int							m_randomSeed;
};



#endif // GLOBALDATA_H_
