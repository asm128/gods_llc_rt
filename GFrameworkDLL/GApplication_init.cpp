//#define GPROFILE_ENABLED
//#define DEBUG_PRINTF_ENABLED
//#define DATA_PRINTF_ENABLED
//#define WARNING_PRINTF_ENABLED
//#define ERROR_PRINTF_ENABLED
//#define RENDER_PRINTF_ENABLED

/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2013: Pablo Ariel Zorrilla Cepeda
#include "GApplication.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"


using namespace god;

CGApplication::CGApplication( void )
{
	GPROFILE_METHOD( "CGApplication", "CGApplication" );
	// Initialize here your member stuff
};

CGApplication::~CGApplication( void )
{
	GPROFILE_METHOD( "CGApplication", "~CGApplication" );
	// Remember to destroy or release everything you created
};

//------------------------------------------------------------
// make some initial configuration for the GUI and everything
// else you may need at start
//------------------------------------------------------------
int32_t CGApplication::InitApplication( void )
{
	GPROFILE_METHOD( "CGApplication", "InitApplication" );
	info_printf("Initializing custom application resources.");
	
	// Put your initialization code here...
	int32_t result = 0;
	IGGame* pGame = GetGame();
	if( pGame )
	{
		if( result = pGame->InitGame( this ) )
		{ 
			error_printf("IGGame::InitGame() returned 0x%X", result);
			return -1;
		}
	}
	else
	{
		warning_printf("Game instance not found (failed to load dll?). This application won't do anything significant without it.");
	}


	info_printf("Finished initializing custom application resources.");
	return 0;
};


// 
int32_t CGApplication::ShutdownApplication( void )
{
	GPROFILE_METHOD( "CGApplication", "ShutdownApplication" );
	info_printf("Shutting down application...");

	// Put your cleanup code here...
	error_t result = 0;
	IGGame* pGame = GetGame();
	if( pGame )
	{
		if( result = pGame->ShutdownGame() )
		{
			error_printf("IGGame::ShutdownGame() returned 0x%X", result);
		}
	}
	else
	{
		if( result = ShutdownFramework() )
		{
			error_printf("CGFramework::ShutdownFramework() returned 0x%X", result);
			return result;
		}
		else
		{
			info_printf( "CGFramework::ShutdownFramework() succeeded." );
		}
	}
	return result;
};