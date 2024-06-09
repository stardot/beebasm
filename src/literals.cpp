/*************************************************************************************************/
/**
	literals.cpp


	Copyright (C) Rich Talbot-Watkins, Charles Reilly 2007-2022

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

#include <cassert>
#include <cstdlib>

#include "asmexception.h"
#include "literals.h"
#include "stringutils.h"

static int hex_digit_value(char c)
{
	if ( '0' <= c && c <= '9' )
	{
		return c - '0';
	}
	else if ( 'A' <= c && c <= 'F' )
	{
		return c - 'A' + 10;
	}
	else if ( 'a' <= c && c <= 'f' )
	{
		return c - 'a' + 10;
	}
	else
	{
		return -1;
	}
}

// Parse a binary or hex integer, return false if it is malformed or too big
static bool ParseInteger(const std::string& line, size_t& index, int base, int max_digits, double& result)
{
	size_t start_column = index;

	// Check there's something and it doesn't start with an underscore
	if (index == line.length() || line[index] == '_')
	{
		return false;
	}

	// Skip leading zeroes
	while ( index < line.length() && (line[index] == '0' || line[index] == '_') )
	{
		index++;
	}

	unsigned int value = 0;
	int digit_count = 0;

	while ( index < line.length() )
	{
		if (line[index] == '_')
		{
			// Don't allow two in a row; there must be a previous char because
			// we can't start with an underscore
			if (line[index - 1] == '_')
			{
				return false;
			}
		}
		else
		{
			int digit = hex_digit_value(line[index]);
			if ( digit < 0 || digit >= base )
			{
				break;
			}
			value = ( value * base ) + digit;
			digit_count++;
		}
		index++;
	}

	// Check we found something, it wasn't too long and it didn't end on an underscore
	if ( index == start_column || digit_count > max_digits || line[index - 1] == '_')
	{
		return false;
	}

	result = static_cast< double >( value );

	return true;
}

// Copy decimal digits to a buffer, skipping single underscores that appear between digits.
// Throw an exception for underscores at the beginning or end or paired.
// Return false if there were no digits.
static bool CopyDigitsSkippingUnderscores(const std::string& line, size_t& index, std::string& buffer)
{
	if ( index < line.length() && line[index] == '_')
	{
		// Number can't start with an underscore
		throw AsmException_SyntaxError_InvalidCharacter( line, index );
	}

	size_t start_index = index;
	while ( index < line.length() && (Ascii::IsDigit(line[index]) || line[index] == '_') )
	{
		if (line[index] == '_')
		{
			// Don't allow two in a row; there must be a previous char because
			// we can't start with an underscore
			if (line[index - 1] == '_')
			{
				throw AsmException_SyntaxError_InvalidCharacter( line, index );
			}
		}
		else
		{
			buffer.push_back(line[index]);
		}
		index++;
	}
	if ( index > start_index && line[index - 1] == '_')
	{
		// Can't end on an underscore
		throw AsmException_SyntaxError_InvalidCharacter( line, index );
	}
	return index != start_index;
}

/*************************************************************************************************/
/**
	Literals::ParseNumeric()

	Parses a simple numeric value.  This may be
	- a decimal literal
	- a hex literal (prefixed by & or $)
	- a binary literal (prefixed by %)

	Any two digits may be separated by a single underscore, which will be ignored.

	Returns false if there isn't a numeric literal, throws an exception if it is malformed
*/
/*************************************************************************************************/
bool Literals::ParseNumeric(const std::string& line, size_t& index, double& result)
{
	if ( index >= line.length() )
	{
		return false;
	}

	if ( Ascii::IsDigit(line[index]) || line[index] == '.' || line[index] == '-' )
	{
		// Copy the number without underscores to this buffer
		std::string buffer;

		if ( line[index] == '-' )
		{
			buffer.push_back('-');
			index++;
		}

		// Copy digits preceding decimal point
		bool have_digits = CopyDigitsSkippingUnderscores(line, index, buffer);

		// Copy decimal point
		if ( index < line.length() && line[index] == '.')
		{
			buffer.push_back(line[index]);
			index++;

			// Copy digits after decimal point
			have_digits = CopyDigitsSkippingUnderscores(line, index, buffer) || have_digits;
		}

		if ( !have_digits )
		{
			// A decimal point with no number will cause this
			throw AsmException_SyntaxError_InvalidCharacter( line, index );
		}

		// Copy exponent if it's followed by a sign or digit
		if ( index + 1 < line.length() &&
			( line[index] == 'e' || line[index] == 'E' ) &&
			( line[index + 1] == '+' || line[index + 1] == '-' || Ascii::IsDigit(line[index + 1]) ) )
		{
			buffer.push_back('e');
			index++;
			if ( line[index] == '+' || line[index] == '-' )
			{
				buffer.push_back(line[index]);
				index++;
			}
			if (!CopyDigitsSkippingUnderscores(line, index, buffer))
			{
				// Exponent needs a value
				throw AsmException_SyntaxError_InvalidCharacter( line, index );
			}
		}

		char* end_ptr;
		result = strtod(buffer.c_str(), &end_ptr);
		assert(*end_ptr == 0);

		return true;
	}
	else if ( (line[index] == '&') || (line[index] == '$') )
	{
		// get hexadecimal

		// skip the number prefix
		index++;

		if (!ParseInteger(line, index, 16, 8, result))
		{
			// badly formed hex literal
			throw AsmException_SyntaxError_BadHex( line, index );
		}

		return true;
	}
	else if ( line[index] == '%' )
	{
		// get binary

		// skip the number prefix
		index++;

		if (!ParseInteger(line, index, 2, 32, result))
		{
			// badly formed bin literal
			throw AsmException_SyntaxError_BadBin( line, index );
		}

		return true;
	}

	return false;
}
