/*************************************************************************************************/
/**
	stringutils.cpp


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

#include <iostream>
#include <sstream>
#include "globaldata.h"
#include "stringutils.h"

using namespace std;


namespace StringUtils
{


/*************************************************************************************************/
/**
	EatWhitespace()

	Moves to the first non-whitespace character

	@param		line			String to process
	@param		column			Character within the string to start from

	@return		bool			Whether there were more non-whitespace characters found
				column is modified to the next non-whitespace character, or the end of the string
*/
/*************************************************************************************************/
bool EatWhitespace( const string& line, size_t& column )
{
	size_t newColumn = line.find_first_not_of( " ", column );
	if ( newColumn == string::npos )
	{
		column = line.length();
		return false;
	}
	else
	{
		column = newColumn;
		return true;
	}
}



/*************************************************************************************************/
/**
	FormattedErrorLocation()

	Return an error location formatted according to the command line options.

	@param		filename		Filename
	@param		lineNumber		Line number

	@return		string			Error location string
*/
/*************************************************************************************************/
std::string FormattedErrorLocation( const std::string& filename, int lineNumber )
{
	std::stringstream s;
	if ( GlobalData::Instance().UseVisualCppErrorFormat() )
	{
		s << filename << "(" << lineNumber << ")";
	}
	else
	{
		s << filename << ":" << lineNumber;
	}
	return s.str();
}


/*************************************************************************************************/
/**
	PrintNumber()

	Print a number to a stream ensuring that 32-bit ints are not written in scientific notation

	@param		dest		The stream to write to
	@param		value		The number to write

*/
/*************************************************************************************************/
void PrintNumber(std::ostream& dest, double value)
{
	if ((-4294967296.0 < value) && (value < -0.5))
	{
		unsigned int abs_part = static_cast<unsigned int>(-value);
		if (-static_cast<double>(abs_part) == value)
		{
			dest << '-' << abs_part;
			return;
		}
	}
	else if ((0.0 <= value) && (value < 4294967296.0))
	{
		unsigned int abs_part = static_cast<unsigned int>(value);
		if (static_cast<double>(abs_part) == value)
		{
			dest << abs_part;
			return;
		}
	}

	dest << value;
}

} // namespace StringUtils
