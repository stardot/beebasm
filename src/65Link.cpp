/*************************************************************************************************/
/**
	65Link.cpp

	Saves file(s) to 65Link volume.


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

#include "65Link.h"
#include "asmexception.h"
#include <string.h>
#include <fstream>

struct CharMapping
{
	char ch;
	const char *pCode;
};

static const CharMapping g_aMappings[] = {
    {' ', "_sp",},
	{'!', "_xm",},
	{'"', "_dq",},
	{'#', "_ha",},
	{'$', "_do",},
	{'%', "_pc",},
	{'&', "_am",},
	{'\'', "_sq",},
	{'(', "_rb",},
	{')', "_lb",},
	{'*', "_as",},
	{'+', "_pl",},
	{',', "_cm",},
	{'-', "_mi",},
	{'.', "_pd",},
	{'/', "_fs",},
	{':', "_co",},
	{';', "_sc",},
	{'<', "_st",},
	{'=', "_eq",},
	{'>', "_lt",},
	{'?', "_qm",},
	{'@', "_at",},
	{'[', "_hb",},
	{'\\', "_bs",},
	{']', "_bh",},
	{'^', "_po",},
	{'_', "_un",},
	{'`', "_bq",},
	{'{', "_cb",},
	{'|', "_ba",},
	{'}', "_bc",},
	{'~', "_no",},
	{},
};

static std::string Get65LinkCharFromBBCChar(char ch)
{
	for ( const CharMapping *pMapping = g_aMappings; pMapping->pCode; ++pMapping )
	{
		if ( pMapping->ch == ch )
			return pMapping->pCode;
	}

	return std::string( 1, ch );
}

static std::string Get65LinkNameFromBBCName(const std::string &bbc_name)
{
	std::string result;

	for ( std::string::const_iterator it = bbc_name.begin(); it != bbc_name.end(); ++it )
		result += Get65LinkCharFromBBCChar( *it );

	return result;
}

void Save65LinkFile( const char *pVolumeName,
					 const char *pBBCName,
					 const unsigned char *pAddr,
					 int load,
					 int exec,
					 int len )
{
	char bbcFileDir;
	std::string bbcFileName = pBBCName;
	
	if ( bbcFileName.size() > 2 && bbcFileName[ 1 ] == '.' )
	{
		bbcFileDir = bbcFileName[0];
		bbcFileName = bbcFileName.substr( 2 );
	}
	else
		bbcFileDir = '$';
	
	std::string fileName = pVolumeName;
	
	if ( !fileName.empty() )
	{
		char c = fileName[ fileName.size() - 1 ];
		if ( c != '\\' && c != '/' )
			fileName += '/';
	}

	fileName += Get65LinkCharFromBBCChar( bbcFileDir ) +  Get65LinkNameFromBBCName( bbcFileName );

	static const std::ios_base::openmode MODE = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc;

	{
		std::ofstream dataFile;

		dataFile.open( fileName.c_str(), MODE );
		
		if ( !dataFile )
			throw AsmException_FileError_OpenObj( fileName.c_str() );

		if ( !dataFile.write( reinterpret_cast< const char * >( pAddr ), len ) )
			throw AsmException_FileError_WriteObj( fileName.c_str() );
	}

	std::string leaFileName = fileName + ".lea";
	
	{
		std::ofstream leaFile;

		leaFile.open( leaFileName.c_str(), MODE );

		if ( !leaFile )
			throw AsmException_FileError_OpenLEA( leaFileName.c_str() );

		const unsigned char leaFileData[ 12 ] = {
			( load >> 0 ) & 255, ( load >> 8 ) & 255, ( load >> 16 ) & 255, ( load >> 24 ) & 255,
			( exec >> 0 ) & 255, ( exec >> 8 ) & 255, ( exec >> 16 ) & 255, ( exec >> 24 ) & 255,
			0, 0, 0, 0,			// attributes
		};

		if ( !leaFile.write( reinterpret_cast< const char * >( leaFileData ),
							 sizeof leaFileData ) )
		{
			throw AsmException_FileError_WriteLEA( leaFileName.c_str() );
		}
	}
}
