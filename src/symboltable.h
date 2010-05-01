/*************************************************************************************************/
/**
	symboltable.h
*/
/*************************************************************************************************/

#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

#include <cassert>
#include <cstdlib>
#include <map>
#include <string>


class SymbolTable
{
public:

	static void Create();
	static void Destroy();
	static inline SymbolTable& Instance() { assert( m_gInstance != NULL ); return *m_gInstance; }

	void AddSymbol( const std::string& symbol, double value );
	void ChangeSymbol( const std::string& symbol, double value );
	double GetSymbol( const std::string& symbol ) const;
	bool IsSymbolDefined( const std::string& symbol ) const;
	void RemoveSymbol( const std::string& symbol );


private:

	SymbolTable();
	~SymbolTable();

	std::map<std::string, double>	m_map;

	static SymbolTable*				m_gInstance;
};



#endif // SYMBOLTABLE_H_
