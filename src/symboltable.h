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

	void AddSymbol( const std::string& symbol, double value, bool isLabel = false );
	void ChangeSymbol( const std::string& symbol, double value );
	double GetSymbol( const std::string& symbol ) const;
	bool IsSymbolDefined( const std::string& symbol ) const;
	void RemoveSymbol( const std::string& symbol );

	void Dump() const;


private:

	class Symbol
	{
	public:

		Symbol( double value, bool isLabel ) : m_value( value ), m_isLabel( isLabel ) {}

		void SetValue( double d ) { m_value = d; }
		double GetValue() const { return m_value; }
		bool IsLabel() const { return m_isLabel; }

	private:

		double	m_value;
		bool	m_isLabel;
	};

	SymbolTable();
	~SymbolTable();

	std::map<std::string, Symbol>	m_map;

	static SymbolTable*				m_gInstance;
};



#endif // SYMBOLTABLE_H_
