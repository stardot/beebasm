/*************************************************************************************************/
/**
	main.cpp

	Main entry point for the application
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


using namespace std;


#define VERSION "0.05"


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
	const char* pDiscInputFile = NULL;
	const char* pDiscOutputFile = NULL;

	enum STATES
	{
		READY,
		WAITING_FOR_INPUT_FILENAME,
		WAITING_FOR_DISC_INPUT_FILENAME,
		WAITING_FOR_DISC_OUTPUT_FILENAME,
		WAITING_FOR_BOOT_FILENAME

	} state = READY;

	bool bDumpSymbols = false;

	GlobalData::Create();

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
				else if ( strcmp( argv[i], "-v" ) == 0 )
				{
					GlobalData::Instance().SetVerbose( true );
				}
				else if ( strcmp( argv[i], "-d" ) == 0 )
				{
					bDumpSymbols = true;
				}
				else if ( strcmp( argv[i], "--help" ) == 0 )
				{
					cout << "beebasm " VERSION << endl << endl;
					cout << "Possible options:" << endl;
					cout << " -i <file>      Specify source filename" << endl;
					cout << " -di <file>     Specify a disc image file to be added to" << endl;
					cout << " -do <file>     Specify a disc image file to output" << endl;
					cout << " -boot <file>   Specify a filename to be run by !BOOT on a new disc image" << endl;
					cout << " -v             Verbose output" << endl;
					cout << " -d             Dump all global symbols after assembly" << endl;
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
		}
	}

	if ( state != READY )
	{
		cerr << "Parameter error -" << endl;
		cerr << "Type beebasm -help for syntax" << endl;
		return EXIT_FAILURE;
	}

	// Check parameters

	if ( pInputFile == NULL )
	{
		cerr << "No source file" << endl;
		return EXIT_FAILURE;
	}

	if ( pDiscInputFile != NULL && pDiscOutputFile == NULL ||
		 ( pDiscInputFile != NULL && pDiscOutputFile != NULL && strcmp( pDiscInputFile, pDiscOutputFile ) == 0 ) )
	{
		cerr << "If a disc image file is provided as input, a different filename must be provided as output" << endl;
		return EXIT_FAILURE;
	}


	// All good, start the assembling

	int exitCode = EXIT_SUCCESS;

	SymbolTable::Create();
	ObjectCode::Create();

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
			ObjectCode::Instance().SetPC( 0 );
			ObjectCode::Instance().Clear( 0, 0x10000, false );
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

	if ( !GlobalData::Instance().IsSaved() )
	{
		cerr << "warning: no SAVE command in source file." << endl;
	}

	ObjectCode::Destroy();
	SymbolTable::Destroy();
	GlobalData::Destroy();

	return exitCode;
}
