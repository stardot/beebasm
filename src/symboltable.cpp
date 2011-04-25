/*************************************************************************************************/
/**
	symboltable.cpp


	Copyright (C) Rich Talbot-Watkins 2007 - 2011

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

#include "symboltable.h"


using namespace std;


SymbolTable* SymbolTable::m_gInstance = NULL;


/*************************************************************************************************/
/**
	SymbolTable::Create()

	Creates the SymbolTable singleton
*/
/*************************************************************************************************/
void SymbolTable::Create()
{
	assert( m_gInstance == NULL );

	m_gInstance = new SymbolTable;
}



/*************************************************************************************************/
/**
	SymbolTable::Destroy()

	Destroys the SymbolTable singleton
*/
/*************************************************************************************************/
void SymbolTable::Destroy()
{
	assert( m_gInstance != NULL );

	delete m_gInstance;
	m_gInstance = NULL;
}



/*************************************************************************************************/
/**
	SymbolTable::SymbolTable()

	SymbolTable constructor
*/
/*************************************************************************************************/
SymbolTable::SymbolTable()
{
	// Add any constant symbols here

	AddSymbol( "PI", M_PI );
	AddSymbol( "P%", 0 );
	AddSymbol( "TRUE", -1 );
	AddSymbol( "FALSE", 0 );
}



/*************************************************************************************************/
/**
	SymbolTable::~SymbolTable()

	SymbolTable destructor
*/
/*************************************************************************************************/
SymbolTable::~SymbolTable()
{
}



/*************************************************************************************************/
/**
	SymbolTable::IsSymbolDefined()

	Returns whether or not the supplied symbol exists in the symbol table

	@param		symbol			The symbol to search for
	@returns	bool
*/
/*************************************************************************************************/
bool SymbolTable::IsSymbolDefined( const std::string& symbol ) const
{
	return ( m_map.count( symbol ) == 1 );
}



/*************************************************************************************************/
/**
	SymbolTable::AddSymbol()

	Adds a symbol to the symbol table with the supplied value

	@param		symbol			The symbol to add
	@param		int				Its value
*/
/*************************************************************************************************/
void SymbolTable::AddSymbol( const std::string& symbol, double value, bool isLabel )
{
	assert( !IsSymbolDefined( symbol ) );
	m_map.insert( make_pair( symbol, Symbol( value, isLabel ) ) );
}



/*************************************************************************************************/
/**
	SymbolTable::GetSymbol()

	Gets the value of a symbol which already exists in the symbol table

	@param		symbol			The name of the symbol to look for
*/
/*************************************************************************************************/
double SymbolTable::GetSymbol( const std::string& symbol ) const
{
	assert( IsSymbolDefined( symbol ) );
	return m_map.find( symbol )->second.GetValue();
}



/*************************************************************************************************/
/**
	SymbolTable::ChangeSymbol()

	Changes the value of a symbol which already exists in the symbol table

	@param		symbol			The name of the symbol to look for
	@param		value			Its new value
*/
/*************************************************************************************************/
void SymbolTable::ChangeSymbol( const std::string& symbol, double value )
{
	assert( IsSymbolDefined( symbol ) );
	m_map.find( symbol )->second.SetValue( value );
}



/*************************************************************************************************/
/**
	SymbolTable::RemoveSymbol()

	Removes the named symbol

	@param		symbol			The name of the symbol to look for
*/
/*************************************************************************************************/
void SymbolTable::RemoveSymbol( const std::string& symbol )
{
	assert( IsSymbolDefined( symbol ) );
	m_map.erase( symbol );
}



/*************************************************************************************************/
/**
	SymbolTable::Dump()

	Dumps all global symbols in the symbol table
*/
/*************************************************************************************************/
void SymbolTable::Dump() const
{
	cout << "[{";

	bool bFirst = true;

	for ( map<string, Symbol>::const_iterator it = m_map.begin(); it != m_map.end(); ++it )
	{
		const string&	symbolName = it->first;
		const Symbol&	symbol = it->second;

		if ( symbol.IsLabel() &&
			 symbolName.find_first_of( '@' ) == string::npos )
		{
			if ( !bFirst )
			{
				cout << ",";
			}

			cout << "'" << symbolName << "':" << symbol.GetValue() << "L";

			bFirst = false;
		}
	}

	cout << "}]" << endl;
}
