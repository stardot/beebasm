/*************************************************************************************************/
/**
	stringutils.h


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

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string>


namespace StringUtils
{
	bool EatWhitespace( const std::string& line, size_t& column );
	std::string FormattedErrorLocation ( const std::string& filename, int lineNumber );
	void PrintNumber(std::ostream& dest, double value);
}


// Built-in char functions like isdigit, toupper, etc. are locale dependent and quite slow.
struct Ascii
{
	static bool IsAlpha(char c)
	{
		unsigned int uc = static_cast<unsigned char>(c);
		return (uc | 0x20) - 'a' < 26;
	}

	static bool IsDigit(char c)
	{
		unsigned int uc = static_cast<unsigned char>(c);
		return (uc - '0') < 10;
	}

	static char ToLower(char c)
	{
		unsigned int uc = static_cast<unsigned char>(c);
		if (uc - 'A' < 26)
		{
			return c | 0x20;
		}
		else
		{
			return c;
		}
	}

	static char ToUpper(char c)
	{
		unsigned int uc = static_cast<unsigned char>(c);
		if (uc - 'a' < 26)
		{
			return c & 0xDF;
		}
		else
		{
			return c;
		}
	}
};


#endif // STRINGUTILS_H_
