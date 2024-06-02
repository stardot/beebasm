/*************************************************************************************************/
/**
	symboltable.h


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

#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

#include <cassert>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <vector>

#include "scopedsymbolname.h"
#include "value.h"


class SymbolTable
{
public:

	static void Create();
	static void Destroy();
	static inline SymbolTable& Instance() { assert( m_gInstance != NULL ); return *m_gInstance; }

	void AddBuiltInSymbol( const std::string& symbol, Value value );
	void AddSymbol( const ScopedSymbolName& symbol, Value value, bool isLabel = false );
	bool AddCommandLineSymbol( const std::string& expr );
	bool AddCommandLineStringSymbol( const std::string& expr );
	void ChangeBuiltInSymbol( const std::string& symbol, double value );
	void ChangeSymbol( const ScopedSymbolName& symbol, double value );
	Value GetSymbol( const ScopedSymbolName& symbol ) const;
	bool IsSymbolDefined( const ScopedSymbolName& symbol ) const;
	void RemoveSymbol( const ScopedSymbolName& symbol );

	void Dump(bool global, bool all, const char * labels_file) const; // labels_file == nullptr -> stdout

	void PushBrace();
	void PushFor(const ScopedSymbolName& symbol, double value);
	void AddLabel(const std::string & symbol);
	void PopScope();

private:

	class Symbol
	{
	public:

		Symbol( Value value, bool isLabel ) : m_value( value ), m_isLabel( isLabel ) {}

		void SetValue( Value value ) { m_value = value; }
		Value GetValue() const { return m_value; }
		bool IsLabel() const { return m_isLabel; }

	private:

		Value	m_value;
		bool	m_isLabel;
	};

	SymbolTable();
	~SymbolTable();

	typedef std::unordered_map<ScopedSymbolName, Symbol> MapType;
	MapType m_map;

	static SymbolTable*				m_gInstance;

	int m_labelScopes;
	struct Label
	{
		int         m_addr;
		int         m_scope;
		std::string m_identifier; // "" -> using label from parent scope
		Label(int addr = 0, int scope = 0, const std::string & identifier = "") : m_addr(addr), m_scope(scope), m_identifier(identifier) {}
	} m_lastLabel;
	std::vector<Label> m_labelStack;
	std::vector<Label> m_labelList;
};



#endif // SYMBOLTABLE_H_
