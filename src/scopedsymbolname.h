/*************************************************************************************************/
/**
	scopedsymbolname.h


	Copyright (C) Charles Reilly 2024

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

#ifndef SCOPEDSYMBOLNAME_H_
#define SCOPEDSYMBOLNAME_H_

#include <string>


class ScopedSymbolName
{
public:
	explicit ScopedSymbolName(const std::string& name) : m_name(name), m_id(-1), m_count(-1)
	{
	}

	ScopedSymbolName(const std::string& name, int id, int count) : m_name(name), m_id(id), m_count(count)
	{
	}

	ScopedSymbolName()
	{
	}

	std::string Name() const
	{
		return m_name;
	}

	bool TopLevel() const
	{
		return m_id == -1;
	}

	bool operator< (const ScopedSymbolName& that) const
	{
		if (m_name < that.m_name)
		{
			return true;
		}
		if (m_name > that.m_name)
		{
			return false;
		}
		if (m_id < that.m_id)
		{
			return true;
		}
		if (m_id > that.m_id)
		{
			return false;
		}
		if (m_count < that.m_count)
		{
			return true;
		}
		return false;
	}

private:

	// The symbol name
	std::string m_name;
	// The scope identifier
	int m_id;
	// The for loop count (number of times through, not current value)
	int m_count;

};



#endif // SCOPEDSYMBOLNAME_H_