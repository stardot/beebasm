/*************************************************************************************************/
/**
	symboltable.cpp
*/
/*************************************************************************************************/

#include <cmath>
#include "symboltable.h"

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
void SymbolTable::AddSymbol( const std::string& symbol, double value )
{
	assert( !IsSymbolDefined( symbol ) );
	m_map.insert( make_pair( symbol, value ) );
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
	return m_map.find( symbol )->second;
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
	m_map.find( symbol )->second = value;
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
