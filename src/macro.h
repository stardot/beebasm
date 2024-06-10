/*************************************************************************************************/
/**
	macro.h


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

#ifndef MACRO_H_
#define MACRO_H_

#include <cassert>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include "sourcecode.h"


class Macro
{
public:

	Macro( const std::string& filename, int lineNumber );


	void SetName( const std::string& name )
	{
		m_name = name;
	}

	void AddParameter( const std::string& param )
	{
		m_parameters.push_back( param );
	}

	void AddLine( const std::string& line )
	{
		m_body += line;
	}

	const std::string& GetName() const
	{
		return m_name;
	}

	int GetNumberOfParameters() const
	{
		return m_parameters.size();
	}

	const std::string& GetParameter( int i ) const
	{
		return m_parameters[ i ];
	}

	const std::string& GetBody() const
	{
		return m_body;
	}

	const std::string& GetFilename() const
	{
		return m_filename;
	}

	int GetLineNumber() const
	{
		return m_lineNumber;
	}


private:

	std::string						m_filename;
	int								m_lineNumber;

	std::string						m_name;
	std::vector< std::string >		m_parameters;
	std::string						m_body;

};


class MacroInstance : public SourceCode
{
public:

	MacroInstance( const Macro* macro, const SourceCode* parent );
	virtual ~MacroInstance() {}
};



class MacroTable
{
public:

	static void Create();
	static void Destroy();
	static inline MacroTable& Instance() { assert( m_gInstance != NULL ); return *m_gInstance; }

	void Add( Macro* macro );
	bool Exists( const std::string& name ) const;
	const Macro* Get( const std::string& name ) const;

private:

	MacroTable();
	~MacroTable();

	std::map< std::string, Macro* >	m_map;

	static MacroTable*				m_gInstance;
};



#endif // MACRO_H_
