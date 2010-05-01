/*************************************************************************************************/
/**
	globaldata.cpp
*/
/*************************************************************************************************/

#include "globaldata.h"

GlobalData* GlobalData::m_gInstance = NULL;



/*************************************************************************************************/
/**
	GlobalData::Create()

	Creates the GlobalData singleton
*/
/*************************************************************************************************/
void GlobalData::Create()
{
	assert( m_gInstance == NULL );

	m_gInstance = new GlobalData;
}



/*************************************************************************************************/
/**
	GlobalData::Destroy()

	Destroys the GlobalData singleton
*/
/*************************************************************************************************/
void GlobalData::Destroy()
{
	assert( m_gInstance != NULL );

	delete m_gInstance;
	m_gInstance = NULL;
}



/*************************************************************************************************/
/**
	GlobalData::GlobalData()

	GlobalData constructor
*/
/*************************************************************************************************/
GlobalData::GlobalData()
	:	m_pBootFile( NULL ),
		m_bVerbose( false ),
		m_bUseDiscImage( false ),
		m_pDiscImage( NULL )
{
}



/*************************************************************************************************/
/**
	GlobalData::~GlobalData()

	GlobalData destructor
*/
/*************************************************************************************************/
GlobalData::~GlobalData()
{
}
