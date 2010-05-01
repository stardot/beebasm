/*************************************************************************************************/
/**
	stringutils.h
*/
/*************************************************************************************************/

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string>


namespace StringUtils
{
	void ExpandTabsToSpaces( std::string& line, size_t tabWidth );
	bool EatWhitespace( const std::string& line, size_t& column );
}


#endif // STRINGUTILS_H_
