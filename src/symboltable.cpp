/*************************************************************************************************/
/**
	symboltable.cpp


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

#include <cmath>
#include <iostream>
#include <sstream>

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
	SymbolTable::AddCommandLineSymbol()

	Adds a symbol to the symbol table using a command line 'FOO=BAR' expression

	@param		expr			Symbol name and value
	@returns	bool
*/
/*************************************************************************************************/
bool SymbolTable::AddCommandLineSymbol( const std::string& expr )
{
	std::string::size_type equalsIndex = expr.find( '=' );
	std::string symbol;
	std::string valueString;
	if ( equalsIndex == std::string::npos )
	{
		symbol = expr;
		valueString = "-1";
	}
	else
	{
		symbol = expr.substr( 0, equalsIndex );
		valueString = expr.substr( equalsIndex + 1 );
	}
	if ( symbol.empty() )
	{
		return false;
	}
	for ( std::string::size_type i = 0; i < symbol.length(); ++i )
	{
		bool valid = ( isalpha( symbol[ i ] ) || ( symbol[ i ] == '_' ) );
		valid = valid || ( ( i > 0 ) && isdigit( symbol[ i ] ) );
		if ( !valid )
		{
			return false;
		}
	}
	if ( IsSymbolDefined( symbol ) )
	{
		return false;
	}

	bool readHex = false;
	bool readBinary = false;

	if ( !valueString.compare( 0, 1, "&" ) || !valueString.compare( 0, 1, "$" ) )
	{
		readHex = true;
		valueString = valueString.substr( 1 );
	}
	else if ( !valueString.compare( 0, 2, "0x" ) || !valueString.compare( 0, 2, "0X" ) )
	{
		readHex = true;
		valueString = valueString.substr( 2 );
	}
	else if ( !valueString.compare( 0, 1, "%" ) )
	{
		readBinary = true;
		valueString = valueString.substr( 1 );
	}

	std::istringstream valueStream( valueString );
	double value;
	char c;

	valueStream >> noskipws;

	if ( readHex )
	{
		int intValue;

		if ( ! ( valueStream  >> hex >> intValue ) )
		{
			return false;
		}

		value = intValue;
	}
	else if ( readBinary )
	{
		unsigned int intValue = 0;

		int charOrEof = valueStream.get();

		if ( charOrEof == EOF )
		{
			return false;
		}

		while ( ( charOrEof == '0' ) || ( charOrEof == '1' ) )
		{
			if ( intValue & 0x80000000 )
			{
				return false;
			}
			intValue = 2 * intValue + (charOrEof - '0');
			charOrEof = valueStream.get();
		}

		if ( charOrEof != EOF )
		{
			return false;
		}

		value = (int)intValue;
	}
	else if ( ! ( valueStream >> value ) )
	{
		return false;
	}

	if ( valueStream.get( c ) )
	{
		return false;
	}

	m_map.insert( make_pair( symbol, Symbol( value, false ) ) );

	return true;
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
