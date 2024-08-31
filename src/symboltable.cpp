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
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "globaldata.h"
#include "objectcode.h"
#include "symboltable.h"
#include "constants.h"
#include "asmexception.h"
#include "literals.h"
#include "stringutils.h"


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
	:	m_labelScopes( 0 )
{
	// Add any constant symbols here

	AddBuiltInSymbol( "PI", const_pi );
	AddBuiltInSymbol( "P%", 0 );
	AddBuiltInSymbol( "TRUE", -1 );
	AddBuiltInSymbol( "FALSE", 0 );
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
bool SymbolTable::IsSymbolDefined( const ScopedSymbolName& symbol ) const
{
	return m_map.find( symbol ) != m_map.cend();
}



/*************************************************************************************************/
/**
	SymbolTable::AddBuiltInSymbol()

	Adds a unscoped symbol to the symbol table with the supplied value

	@param		symbol			The symbol to add
	@param		value			Its value
*/
/*************************************************************************************************/
void SymbolTable::AddBuiltInSymbol( const string& name, Value value )
{
	AddSymbol(ScopedSymbolName(name), value, false);
}



/*************************************************************************************************/
/**
	SymbolTable::AddSymbol()

	Adds a scoped symbol to the symbol table with the supplied value

	@param		symbol			The symbol to add
	@param		value			Its value
*/
/*************************************************************************************************/
void SymbolTable::AddSymbol( const ScopedSymbolName& symbol, Value value, bool isLabel )
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
		bool valid = ( Ascii::IsAlpha( symbol[ i ] ) || ( symbol[ i ] == '_' ) );
		valid = valid || ( ( i > 0 ) && Ascii::IsDigit( symbol[ i ] ) );
		if ( !valid )
		{
			return false;
		}
	}
	if ( IsSymbolDefined( ScopedSymbolName(symbol) ) )
	{
		return false;
	}

	// Convert C-style hex prefix to beeb-style
	if ( !valueString.compare( 0, 2, "0x" ) || !valueString.compare( 0, 2, "0X" ) )
	{
		valueString = valueString.substr( 1 );
		valueString[0] = '&';
	}

	size_t index = 0;
	double value;
	try
	{
		if ( !Literals::ParseNumeric(valueString, index, value) )
		{
			return false;
		}
	}
	catch (AsmException_SyntaxError const&)
	{
		return false;
	}

	if (index != valueString.length())
	{
		return false;
	}

	m_map.insert( make_pair( symbol, Symbol( value, false ) ) );

	return true;
}



/*************************************************************************************************/
/**
	SymbolTable::AddCommandLineStringSymbol()

	Adds a string symbol to the symbol table using a command line 'FOO=BAR' expression

	@param		expr			Symbol name and value
	@returns	bool
*/
/*************************************************************************************************/
bool SymbolTable::AddCommandLineStringSymbol( const std::string& expr )
{
	std::string::size_type equalsIndex = expr.find( '=' );
	std::string symbol;
	std::string valueString;
	if ( equalsIndex == std::string::npos )
	{
		return false;
	}

	symbol = expr.substr( 0, equalsIndex );
	valueString = expr.substr( equalsIndex + 1 );

	if ( symbol.empty() )
	{
		return false;
	}

	String value = String(valueString.data(), valueString.length());

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
Value SymbolTable::GetSymbol( const ScopedSymbolName& symbol ) const
{
	assert( IsSymbolDefined( symbol ) );
	return m_map.find( symbol )->second.GetValue();
}



/*************************************************************************************************/
/**
	SymbolTable::ChangeBuiltInSymbol()

	Changes the value of an unscoped symbol which already exists in the symbol table

	@param		symbolName		The name of the symbol to look for
	@param		value			Its new value
*/
/*************************************************************************************************/
void SymbolTable::ChangeBuiltInSymbol( const std::string& symbolName, double value )
{
	ChangeSymbol(ScopedSymbolName(symbolName), value);
}



/*************************************************************************************************/
/**
	SymbolTable::ChangeSymbol()

	Changes the value of a scoped symbol which already exists in the symbol table

	@param		symbol			The name and scope of the symbol to look for
	@param		value			Its new value
*/
/*************************************************************************************************/
void SymbolTable::ChangeSymbol( const ScopedSymbolName& symbol, Value value )
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
void SymbolTable::RemoveSymbol( const ScopedSymbolName& symbol )
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
void SymbolTable::Dump(bool global, bool all, const char * labels_file) const
{
	std::ofstream labels;
	std::ostream & our_cout = (labels_file && (labels.open(labels_file), !labels.bad())) ? labels : std::cout;

	our_cout << "[{";

	bool bFirst = true;

	if (global)
	{
		typedef vector< pair<double, ScopedSymbolName> > ListType;
		ListType list;

		for ( MapType::const_iterator it = m_map.begin(); it != m_map.end(); ++it )
		{
			const ScopedSymbolName&	symbolName = it->first;
			const Symbol&	symbol = it->second;

			if ( symbol.IsLabel() &&
				 symbolName.TopLevel() )
			{
				// This doesn't output string valued symbols
				Value value = symbol.GetValue();
				if (value.GetType() == Value::NumberValue)
				{
					list.push_back( ListType::value_type(value.GetNumber(), symbolName) );
				}
			}
		}
		sort(list.begin(), list.end());
		for ( ListType::const_iterator it = list.begin(); it != list.end(); ++it )
		{
			if ( it != list.begin() )
			{
				our_cout << ",";
			}

			our_cout << "'" << it->second.Name() << "':" << it->first << "L";
		}
	}

	if (all)
	{
		for ( std::vector<Label>::const_iterator it = m_labelList.begin(); it != m_labelList.end(); ++it )
		{
			if ( !bFirst )
			{
				our_cout << ",";
			}

			our_cout << "'" << it->m_identifier << "':" << it->m_addr << "L";

			bFirst = false;
		}
	}

	our_cout << "}]" << endl;
}

void SymbolTable::PushBrace()
{
	if (GlobalData::Instance().IsSecondPass())
	{
		int addr = ObjectCode::Instance().GetPC();
		if (m_lastLabel.m_addr != addr)
		{
			std::ostringstream label; label << "._" << (m_labelScopes - m_lastLabel.m_scope);
			m_lastLabel.m_identifier = (m_labelStack.empty() ? "" : m_labelStack.back().m_identifier) + label.str();
			m_lastLabel.m_addr = addr;
		}
		m_lastLabel.m_scope = m_labelScopes++;
		m_labelStack.push_back(m_lastLabel);
	}
}

void SymbolTable::PushFor(const ScopedSymbolName& symbol, double value)
{
	if (GlobalData::Instance().IsSecondPass())
	{
		int addr = ObjectCode::Instance().GetPC();
		std::ostringstream label; label << "._" << symbol.Name() << "_" << value;
		m_lastLabel.m_identifier += label.str();
		m_lastLabel.m_addr  = addr;
		m_lastLabel.m_scope = m_labelScopes++;
		m_labelStack.push_back(m_lastLabel);
	}
}

void SymbolTable::AddLabel(const std::string& symbol)
{
	if (GlobalData::Instance().IsSecondPass())
	{
		int addr = ObjectCode::Instance().GetPC();
		m_lastLabel.m_identifier = (m_labelStack.empty() ? "" : m_labelStack.back().m_identifier) + "." + symbol;
		m_lastLabel.m_addr = addr;
		m_labelList.push_back(m_lastLabel);
	}
}

void SymbolTable::PopScope()
{
	if (GlobalData::Instance().IsSecondPass())
	{
		m_labelStack.pop_back();
		m_lastLabel = m_labelStack.empty() ? Label() : m_labelStack.back();
	}
}
