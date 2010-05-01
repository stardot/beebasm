/*************************************************************************************************/
/**
	stringutils.cpp
*/
/*************************************************************************************************/

#include <iostream>
#include "stringutils.h"

using namespace std;


namespace StringUtils
{


/*************************************************************************************************/
/**
	ExpandTabsToSpaces()

	Globally replaces all tab characters in the string with spaces
*/
/*************************************************************************************************/
void ExpandTabsToSpaces( string& line, size_t tabWidth )
{
	(void)tabWidth;
	size_t index;

	while ( ( index = line.find( '\t' ) ) != string::npos )
	{
		// use the below if you want to align to tab stops
		// line.replace( index, 1, string( tabWidth - index % tabWidth, ' ' ) );

		line.replace( index, 1, " " );
	}

	// This method is now misnamed as I now also process \r\n to become just \n.
	// This means it might not work on every platform, but I had to do this because of a
	// bug in MinGW regarding handling of files opened in 'text' mode.
	while ( ( index = line.find( '\r' ) ) != string::npos )
	{
		line.replace( index, 1, "" );
	}
}



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
	size_t newColumn = line.find_first_not_of( " \t\r\n", column );
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



} // namespace StringUtils
