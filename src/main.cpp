/*************************************************************************************************/
/**
	main.cpp

	Main entry point for the application


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
#include <cstring>
#include <cstdlib>
#include <ctime>

#include "main.h"
#include "sourcefile.h"
#include "asmexception.h"
#include "globaldata.h"
#include "objectcode.h"
#include "symboltable.h"
#include "discimage.h"
#include "BASIC.h"
#include "macro.h"
#include "mostream.h"


using namespace std;


#define VERSION "1.08"

static void StripSpaces( string *s )
{
	while ( !s->empty() && isspace( (*s)[0] ) )
		*s = s->substr( 1 );

	while ( !s->empty() && isspace( (*s)[s->size() - 1] ) )
		*s = s->substr( 0, s->size() - 1 );
}

static bool GetHexInt( double *d, const string &s )
{
	char *ep;
	unsigned long ul = strtoul( s.c_str(), &ep, 16);

	if ( *ep != 0 )
		return false;

	*d = ul;
	return true;
}

static bool ParseCommandLineVariableDefinition( string *var, double *val, const char *arg )
{
	const char *eq = strchr( arg, '=' );
	if ( !eq )
		return false;

	*var = string( arg, eq );
	string valStr( eq + 1 );

	StripSpaces( var );
	StripSpaces( &valStr );

	if ( var->empty() )
		return false;

	if ( valStr[0] == '$' || valStr[0] == '&')
	{
		if ( !GetHexInt( val, valStr.substr( 1 ) ) )
			return false;
	}
	else if ( valStr.size() > 2 && valStr[0] == '0' && valStr[1] == 'x')
	{
		if ( !GetHexInt( val, valStr.substr( 2 ) ) )
			return false;
	}
	else
	{
		char *ep;
		*val = strtod( valStr.c_str(), &ep );
		if ( *ep != 0 )
			return false;
	}

	return true;
}

/*************************************************************************************************/
/**
	main()

	The main entry point for the application

	@param		argc			Number of parameters passed
	@param		argv			Array of parameters
*/
/*************************************************************************************************/

int main( int argc, char* argv[] )
{
	const char* pInputFile = NULL;
	const char* pOutputFile = NULL;
	const char* pDiscInputFile = NULL;
	const char* pDiscOutputFile = NULL;

	enum STATES
	{
		READY,
		WAITING_FOR_INPUT_FILENAME,
		WAITING_FOR_OUTPUT_FILENAME,
		WAITING_FOR_DISC_INPUT_FILENAME,
		WAITING_FOR_DISC_OUTPUT_FILENAME,
		WAITING_FOR_BOOT_FILENAME,
		WAITING_FOR_DISC_OPTION,
		WAITING_FOR_VOLUME,
		WAITING_FOR_VERBOSE_OUTPUT_FILENAME

	} state = READY;

	bool bDumpSymbols = false;

	GlobalData::Create();
	SymbolTable::Create();

	// Parse command line parameters

	bool verbose = false;
	const char *pVerboseOutputFilename = 0;

	for ( int i = 1; i < argc; i++ )
	{
		switch ( state )
		{
			case READY:

				if ( strcmp( argv[i], "-i" ) == 0 )
				{
					state = WAITING_FOR_INPUT_FILENAME;
				}
				else if ( strcmp( argv[i], "-o" ) == 0 )
				{
					state = WAITING_FOR_OUTPUT_FILENAME;
				}
				else if ( strcmp( argv[i], "-do" ) == 0 )
				{
					state = WAITING_FOR_DISC_OUTPUT_FILENAME;
				}
				else if ( strcmp( argv[i], "-di" ) == 0 )
				{
					state = WAITING_FOR_DISC_INPUT_FILENAME;
				}
				else if ( strcmp( argv[i], "-boot" ) == 0 )
				{
					state = WAITING_FOR_BOOT_FILENAME;
				}
				else if ( strcmp( argv[i], "-opt" ) == 0 )
				{
					state = WAITING_FOR_DISC_OPTION;
				}
				else if ( strcmp( argv[i], "-v" ) == 0 )
				{
					verbose = true;
				}
				else if ( strcmp( argv[i], "-d" ) == 0 )
				{
					bDumpSymbols = true;
				}
				else if ( strcmp( argv[i], "-pad" ) == 0 )
				{
					GlobalData::Instance().SetPadDiscImage( true );
				}
				else if ( strcmp( argv[i], "-volume" ) == 0 )
				{
					state = WAITING_FOR_VOLUME;
				}
				else if ( argv[i][0] == '-' && argv[i][1] == 'D' )
				{
					string var;
					double val;
					if ( !ParseCommandLineVariableDefinition( &var, &val, argv[i] + 2 ) )
						goto bad_parameter;

					SymbolTable::Instance().AddSymbol(var, val, false);
				}
				else if ( strcmp( argv[i], "-l") == 0 )
				{
					state = WAITING_FOR_VERBOSE_OUTPUT_FILENAME;
				}
				else if ( strcmp( argv[i], "--help" ) == 0 )
				{
					cout << "beebasm " VERSION << endl << endl;
					cout << "Possible options:" << endl;
					cout << " -i <file>      Specify source filename" << endl;
					cout << " -o <file>      Specify output filename (when not specified by SAVE command)" << endl;
					cout << " -di <file>     Specify a disc image file to be added to" << endl;
					cout << " -do <file>     Specify a disc image file to output" << endl;
					cout << " -boot <file>   Specify a filename to be run by !BOOT on a new disc image" << endl;
					cout << " -opt <opt>     Specify the *OPT 4,n for the generated disc image" << endl;
					cout << " -v             Verbose output" << endl;
					cout << " -d             Dump all global symbols after assembly" << endl;
					cout << " -pad           Pad disc image to full size" << endl;
					cout << " -volume <dir>  Specify path to 65Link drive folder to add file to" << endl;
					cout << " -D<var>=<val>  Set global variable <var> to <val>" << endl;
					cout << " --help         See this help again" << endl;
					cout << " -l <file>      Save verbose output to <file>" << endl;
					return EXIT_SUCCESS;
				}
				else
				{
bad_parameter:
					cerr << "Bad parameter: " << argv[i] << endl;
					cerr << "Type beebasm --help for options" << endl;
					return EXIT_FAILURE;
				}
				break;


			case WAITING_FOR_INPUT_FILENAME:

				pInputFile = argv[i];
				state = READY;
				break;


			case WAITING_FOR_OUTPUT_FILENAME:

				pOutputFile = argv[i];
				GlobalData::Instance().SetOutputFile( pOutputFile );
				state = READY;
				break;


			case WAITING_FOR_DISC_OUTPUT_FILENAME:

				pDiscOutputFile = argv[i];
				GlobalData::Instance().SetUseDiscImage( true );
				state = READY;
				break;


			case WAITING_FOR_DISC_INPUT_FILENAME:

				pDiscInputFile = argv[i];
				state = READY;
				break;


			case WAITING_FOR_BOOT_FILENAME:

				GlobalData::Instance().SetBootFile( argv[i] );
				state = READY;
				break;

			case WAITING_FOR_DISC_OPTION:

				GlobalData::Instance().SetDiscOption( std::strtol( argv[i], NULL, 10 ) );
				state = READY;
				break;

			case WAITING_FOR_VOLUME:

				GlobalData::Instance().SetVolume( argv[i] );
				state = READY;
				break;

			case WAITING_FOR_VERBOSE_OUTPUT_FILENAME:

				pVerboseOutputFilename = argv[i];
				state = READY;
				break;
		}
	}

	if ( state != READY )
	{
		cerr << "Parameter error -" << endl;
		cerr << "Type beebasm --help for syntax" << endl;
		return EXIT_FAILURE;
	}

	// Check parameters

	if ( pInputFile == NULL )
	{
		cerr << "No source file" << endl;
		return EXIT_FAILURE;
	}

	if ( ( pDiscInputFile != NULL && pDiscOutputFile == NULL ) ||
		 ( pDiscInputFile != NULL && pDiscOutputFile != NULL && strcmp( pDiscInputFile, pDiscOutputFile ) == 0 ) )
	{
		cerr << "If a disc image file is provided as input, a different filename must be provided as output" << endl;
		return EXIT_FAILURE;
	}

	// Set up the verbose output streams.
	mostream verboseAsmOutputStreams;
	mostream verboseMessageOutputStreams;
	std::ofstream verboseFileOutputStream;

	{
		bool useStreams = false;
	
		if ( verbose )
		{
			verboseAsmOutputStreams.add( &cout );
			verboseMessageOutputStreams.add( &cerr );

			useStreams = true;
		}


		if ( pVerboseOutputFilename )
		{
			verboseFileOutputStream.open( pVerboseOutputFilename, ios_base::out );
			
			if ( !verboseFileOutputStream )
			{
				throw AsmException_FileError_OpenVerboseOutputFile( pVerboseOutputFilename );
			}

			verboseAsmOutputStreams.add( &verboseFileOutputStream );
			verboseMessageOutputStreams.add( &verboseFileOutputStream );

			useStreams = true;
		}

		if ( useStreams )
		{
			GlobalData::Instance().SetVerboseAsmOutputStream( &verboseAsmOutputStreams );
			GlobalData::Instance().SetVerboseMessageOutputStream( &verboseMessageOutputStreams );
		}
	}

	// All good, start the assembling

	int exitCode = EXIT_SUCCESS;

	ObjectCode::Create();
	MacroTable::Create();
	SetupBASICTables();

	time_t randomSeed = time( NULL );

	DiscImage* pDiscIm = NULL;

	try
	{
		if ( GlobalData::Instance().UsesDiscImage() )
		{
			pDiscIm = new DiscImage( pDiscOutputFile, pDiscInputFile );
			GlobalData::Instance().SetDiscImage( pDiscIm );
		}

		for ( int pass = 0; pass < 2; pass++ )
		{
			GlobalData::Instance().SetPass( pass );
			ObjectCode::Instance().InitialisePass();
			GlobalData::Instance().ResetForId();
			srand( static_cast< unsigned int >( randomSeed ) );
			SourceFile input( pInputFile );
			input.Process();
		}
	}
	catch ( AsmException& e )
	{
		e.Print();
		exitCode = EXIT_FAILURE;
	}

	delete pDiscIm;

	if ( bDumpSymbols && exitCode == EXIT_SUCCESS )
	{
		SymbolTable::Instance().Dump();
	}

	if ( !GlobalData::Instance().IsSaved() && exitCode == EXIT_SUCCESS )
	{
		cerr << "warning: no SAVE command in source file." << endl;
	}

	MacroTable::Destroy();
	ObjectCode::Destroy();
	SymbolTable::Destroy();
	GlobalData::Destroy();

	return exitCode;
}
