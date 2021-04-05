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
#include "random.h"


using namespace std;


#define VERSION "1.10-pre"


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
		WAITING_FOR_DISC_TITLE,
		WAITING_FOR_SYMBOL

	} state = READY;

	bool bDumpSymbols = false;

	GlobalData::Create();
	SymbolTable::Create();

	// Parse command line parameters

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
				else if ( strcmp( argv[i], "-title" ) == 0 )
				{
					state = WAITING_FOR_DISC_TITLE;
				}
				else if ( strcmp( argv[i], "-w" ) == 0 )
				{
					GlobalData::Instance().SetRequireDistinctOpcodes( true );
				}
				else if ( strcmp( argv[i], "-vc" ) == 0 )
				{
					GlobalData::Instance().SetUseVisualCppErrorFormat( true );
				}
				else if ( strcmp( argv[i], "-v" ) == 0 )
				{
					GlobalData::Instance().SetVerbose( true );
				}
				else if ( strcmp( argv[i], "-d" ) == 0 )
				{
					bDumpSymbols = true;
				}
				else if ( strcmp( argv[i], "-D" ) == 0 )
				{
					state = WAITING_FOR_SYMBOL;
				}
				else if ( ( strcmp( argv[i], "--help" ) == 0 ) ||
					  ( strcmp( argv[i], "-help" ) == 0 ) ||
					  ( strcmp( argv[i], "-h" ) == 0 ) )
				{
					cout << "beebasm " VERSION << endl << endl;
					cout << "Possible options:" << endl;
					cout << " -i <file>      Specify source filename" << endl;
					cout << " -o <file>      Specify output filename (when not specified by SAVE command)" << endl;
					cout << " -di <file>     Specify a disc image file to be added to" << endl;
					cout << " -do <file>     Specify a disc image file to output" << endl;
					cout << " -boot <file>   Specify a filename to be run by !BOOT on a new disc image" << endl;
					cout << " -opt <opt>     Specify the *OPT 4,n for the generated disc image" << endl;
					cout << " -title <title> Specify the title for the generated disc image" << endl;
					cout << " -v             Verbose output" << endl;
					cout << " -d             Dump all global symbols after assembly" << endl;
					cout << " -w             Require whitespace between opcodes and labels" << endl;
					cout << " -vc            Use Visual C++-style error messages" << endl;
					cout << " -D <sym>=<val> Define symbol prior to assembly" << endl;
					cout << " --help         See this help again" << endl;
					return EXIT_SUCCESS;
				}
				else
				{
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

			case WAITING_FOR_DISC_TITLE:

				if ( strlen( argv[i] ) > 12 )
				{
					cerr << "Disc title cannot be longer than 12 characters" << endl;
					return EXIT_FAILURE;
				}
				GlobalData::Instance().SetDiscTitle( argv[i] );
				state = READY;
                                break;

			case WAITING_FOR_SYMBOL:

				if ( ! SymbolTable::Instance().AddCommandLineSymbol( argv[i] ) )
				{
					cerr << "Invalid -D expression: " << argv[i] << endl;
					return EXIT_FAILURE;
				}
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
			beebasm_srand( static_cast< unsigned long >( randomSeed ) );
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
