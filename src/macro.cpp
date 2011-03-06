/*************************************************************************************************/
/**
	macro.cpp


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
#include <iostream>

#include "macro.h"


using namespace std;


MacroTable* MacroTable::m_gInstance = NULL;


/*************************************************************************************************/
/**
	MacroTable::Create()

	Creates the MacroTable singleton
*/
/*************************************************************************************************/
void MacroTable::Create()
{
	assert( m_gInstance == NULL );

	m_gInstance = new MacroTable;
}



/*************************************************************************************************/
/**
	MacroTable::Destroy()

	Destroys the MacroTable singleton
*/
/*************************************************************************************************/
void MacroTable::Destroy()
{
	assert( m_gInstance != NULL );

	delete m_gInstance;
	m_gInstance = NULL;
}



/*************************************************************************************************/
/**
	MacroTable::MacroTable()

	MacroTable constructor
*/
/*************************************************************************************************/
MacroTable::MacroTable()
{
}



/*************************************************************************************************/
/**
	MacroTable::~MacroTable()

	MacroTable destructor
*/
/*************************************************************************************************/
MacroTable::~MacroTable()
{
	for ( map< std::string, Macro* >::iterator it = m_map.begin(); it != m_map.end(); ++it )
	{
		delete it->second;
	}
}


/*************************************************************************************************/
/**
	MacroTable::Add()

	Adds a new macro to the table
*/
/*************************************************************************************************/
void MacroTable::Add( Macro* macro )
{
	if ( macro != NULL )
	{
		m_map.insert( make_pair( macro->GetName(), macro ) );
	}
}


/*************************************************************************************************/
/**
	MacroTable::Exists()

	Returns whether or not the named macro yet exists
*/
/*************************************************************************************************/
bool MacroTable::Exists( const string& name ) const
{
	return ( m_map.count( name ) > 0 );
}


/*************************************************************************************************/
/**
	MacroTable::Get()

	Returns a reference to the named macro
*/
/*************************************************************************************************/
const Macro* MacroTable::Get( const string& name ) const
{
	if ( Exists( name ) )
	{
		return m_map.find( name )->second;
	}
	else
	{
		return NULL;
	}
}
