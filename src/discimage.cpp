/*************************************************************************************************/
/**
	discimage.cpp


	Copyright (C) Rich Talbot-Watkins 2007, 2008

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

#include <cstdlib>
#include <cstring>
#include "discimage.h"
#include "asmexception.h"
#include "globaldata.h"

using namespace std;


/*************************************************************************************************/
/**
	DiscImage::DiscImage()

	DiscImage constructor
*/
/*************************************************************************************************/
DiscImage::DiscImage( const char* pOutput, const char* pInput )
	:	m_outputFilename( pOutput ),
		m_inputFilename( pInput )
{
	// open output file

	m_outputFile.open( pOutput, ios_base::out | ios_base::binary | ios_base::trunc );

	if ( !m_outputFile )
	{
		throw AsmException_FileError_OpenDiscDest( pOutput );
	}

	// open and load input file if necessary

	if ( pInput != NULL )
	{
		m_inputFile.open( pInput, ios_base::in | ios_base::binary );

		if ( !m_inputFile )
		{
			throw AsmException_FileError_OpenDiscSource( pInput );
		}

		if ( !m_inputFile.read( reinterpret_cast< char* >( m_aCatalog ), 0x200 ) )
		{
			throw AsmException_FileError_ReadDiscSource( pInput );
		}

		// copy the disc contents to the output file

		int endSectorAddr;

		if ( m_aCatalog[ 0x105 ] > 0 )
		{
			int sectorAddrOfLastFile	= m_aCatalog[ 0x10F ] +
										  ( ( m_aCatalog[ 0x10E ] & 0x03 ) << 8 );

			int lengthOfLastFile		= m_aCatalog[ 0x10C ] +
										  ( m_aCatalog[ 0x10D ] << 8 ) +
										  ( ( m_aCatalog[ 0x10E ] & 0x30 ) << 12 );

			endSectorAddr = sectorAddrOfLastFile + ( ( lengthOfLastFile + 0xFF ) >> 8 );
		}
		else
		{
			endSectorAddr = 2;
		}

		m_inputFile.seekg( 0, ios::end );
		int length = m_inputFile.tellg();
		m_inputFile.seekg( 0, ios::beg );

		assert( length >= endSectorAddr * 0x100 );

		char sector[ 0x100 ];

		for ( int sect = 0; sect < endSectorAddr; sect++ )
		{
			if ( !m_inputFile.read( sector, 0x100 ) )
			{
				throw AsmException_FileError_ReadDiscSource( pInput );
			}

			if ( !m_outputFile.write( sector, 0x100 ) )
			{
				throw AsmException_FileError_WriteDiscDest( pOutput );
			}
		}

	}
	else
	{
		// generate a blank catalog

		memset( m_aCatalog, 0, 0x200 );
		m_aCatalog[ 0x106 ] = 0x33;
		m_aCatalog[ 0x107 ] = 0x20;

		if ( !m_outputFile.write( reinterpret_cast< char* >( m_aCatalog ), 0x200 ) )
		{
			// write error
			throw AsmException_FileError_WriteDiscDest( m_outputFilename );
		}

		// add in a boot file

		if ( GlobalData::Instance().GetBootFile() != NULL )
		{
			char pPlingBoot[ 64 ];
			strcpy( pPlingBoot, "*BASIC\r*RUN " );
			strcat( pPlingBoot, GlobalData::Instance().GetBootFile() );
			strcat( pPlingBoot, "\r" );

			AddFile( "!Boot", reinterpret_cast< unsigned char* >( pPlingBoot ), 0, 0xFFFFFF, strlen( pPlingBoot ) );
		}
	}

}



/*************************************************************************************************/
/**
	DiscImage::~DiscImage()

	DiscImage destructor
*/
/*************************************************************************************************/
DiscImage::~DiscImage()
{
	// write back the catalog

	m_outputFile.seekp( 0, ios::beg );
	if ( !m_outputFile.write( reinterpret_cast< char* >( m_aCatalog ), 0x200 ) )
	{
		// don't throw an exception in the destructor, just bear it silently..!
	}

	m_outputFile.close();
	m_inputFile.close();
}



/*************************************************************************************************/
/**
	DiscImage::AddFile()
*/
/*************************************************************************************************/
void DiscImage::AddFile( const char* pName, const unsigned char* pAddr, int load, int exec, int len )
{
	(void)pAddr;

	if ( strlen( pName ) > 7 )
	{
		// Bad name
		throw AsmException_FileError_BadName( m_outputFilename );
	}

	if ( m_aCatalog[ 0x105 ] == 31*8 )
	{
		// Catalog full
		throw AsmException_FileError_TooManyFiles( m_outputFilename );
	}

	// Check the file doesn't already exist

	for ( int i = m_aCatalog[ 0x105 ]; i > 0; i -= 8 )
	{
		bool bTheSame = true;

		for ( size_t j = 0; j < strlen( pName ); j++ )
		{
			if ( toupper( pName[ j ] ) != toupper( m_aCatalog[ i + j ] ) )
			{
				bTheSame = false;
				break;
			}
		}

		if ( bTheSame && ( m_aCatalog[ i + 7 ] & 0x7F ) == '$' )
		{
			// File already exists
			throw AsmException_FileError_FileExists( m_outputFilename );
		}
	}

	// Calculate sector address for the new file

	int sectorAddrOfThisFile;

	if ( m_aCatalog[ 0x105 ] > 0 )
	{
		int sectorAddrOfLastFile	= m_aCatalog[ 0x10F ] +
									  ( ( m_aCatalog[ 0x10E ] & 0x03 ) << 8 );

		int lengthOfLastFile		= m_aCatalog[ 0x10C ] +
									  ( m_aCatalog[ 0x10D ] << 8 ) +
									  ( ( m_aCatalog[ 0x10E ] & 0x30 ) << 12 );

		sectorAddrOfThisFile = sectorAddrOfLastFile + ( ( lengthOfLastFile + 0xFF ) >> 8 );
	}
	else
	{
		sectorAddrOfThisFile = 2;
	}

	int sectorLengthOfThisFile	= ( len + 0xFF ) >> 8;

	if ( sectorAddrOfThisFile + sectorLengthOfThisFile > 800 )
	{
		// Disc full
		throw AsmException_FileError_DiscFull( m_outputFilename );
	}

	// Make space in the catalog for the new file

	for ( int i = m_aCatalog[ 0x105 ]; i > 0; i -= 8 )
	{
		for ( int j = 0; j < 8; j++ )
		{
			m_aCatalog[ i + j + 8 ] = m_aCatalog[ i + j ];
			m_aCatalog[ i + j + 0x108 ] = m_aCatalog[ i + j + 0x100 ];
		}
	}

	// Increment the file count

	m_aCatalog[ 0x105 ] += 8;

	// Write filename

	for ( size_t j = 0; j < 7; j++ )
	{
		m_aCatalog[ j + 8 ] = ( j < strlen( pName ) ) ? pName[ j ] : ' ';
	}

	// Write directory name

	m_aCatalog[ 15 ] = '$';

	// Write load address

	m_aCatalog[ 0x108 ] = load & 0xFF;
	m_aCatalog[ 0x109 ] = ( load & 0xFF00 ) >> 8;

	// Write exec address

	m_aCatalog[ 0x10A ] = exec & 0xFF;
	m_aCatalog[ 0x10B ] = ( exec & 0xFF00 ) >> 8;

	// Write length

	m_aCatalog[ 0x10C ] = len & 0xFF;
	m_aCatalog[ 0x10D ] = ( len & 0xFF00 ) >> 8;

	// Write sector start

	m_aCatalog[ 0x10F ] = sectorAddrOfThisFile & 0xFF;

	// Write miscellaneous bits

	m_aCatalog[ 0x10E ] = ( ( ( load >> 16 ) & 0x03 ) << 2 ) |
						  ( ( ( exec >> 16 ) & 0x03 ) << 6 ) |
						  ( ( ( len  >> 16 ) & 0x03 ) << 4 ) |
						  ( ( sectorAddrOfThisFile >> 8 ) & 0x03 );

	// Now write the actual file

	assert( static_cast< int >( m_outputFile.tellp() ) == sectorAddrOfThisFile * 0x100 );

	if ( !m_outputFile.write( reinterpret_cast< const char* >( pAddr ), len ) )
	{
		// Write error
		throw AsmException_FileError_WriteDiscDest( m_outputFilename );
	}

	while ( ( m_outputFile.tellp() & 0xFF ) != 0 )
	{
		if ( !m_outputFile.put( 0 ) )
		{
			// Write error
			throw AsmException_FileError_WriteDiscDest( m_outputFilename );
		}
	}
}
