//#define GPROFILE_ENABLED
//#define DEBUG_PRINTF_ENABLED
//#define DATA_PRINTF_ENABLED
//#define WARNING_PRINTF_ENABLED
//#define ERROR_PRINTF_ENABLED
//#define RENDER_PRINTF_ENABLED

/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
#include "GApplication.h"
#include <math.h>
#include "GCore_printf.h"
#include "GDebugger_core.h"


using namespace god;

void CalcFrameRate( wchar_t* buffer, uint32_t size, float32_t fElapsedTime, uint32_t Width, uint32_t Height );

//------------------------------------------------------------
// Process WinAPI messages
int32_t CGApplication::ApplicationMsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam )
{
	//int32_t result = 0;
	//IGGame* pGame = GetGame();
	//if( pGame )
	//{
	//	result = pGame->MsgProc(hWnd, uMsg, wParam, lParam);
	//	if( result )
	//	{
	//		error_printf( "IGGame::MsgProc() returned 0x%X", result );
	//	}
	//}
	return 0;
};

//----------- Main loop function calls --------------
//

//------------------------------------------------------------
// Read and process user input.
int32_t CGApplication::ProcessUserInput( float fElapsedTime )
{
	GPROFILE_METHOD( "CGApplication", "ProcessUserInput" );
	GHPTR(Keyboard) pKeyboard = GetKeyboardManager();
	if( pKeyboard->IsKeyDown( GKEY_LALT ) && pKeyboard->IsKeyDown( GKEY_RETURN ) )
	{// ALT + Enter toggles fullscreen mode.
		if( ToggleFullScreen() )
		{
			error_printf( "Request to toggle full screen through hotkey FAILED! (ALT-RETURN)." );
			return -1;
		}
		else
		{	// do not print anything here to prevent spamming the debug string
			// info_printf( "Toggle full screen request handled successfully.\n" );
		}
		return 0;
	}
	if( pKeyboard->IsKeyDown( GKEY_LCONTROL ) && pKeyboard->IsKeyDown( GKEY_Q ) )
	{// CTRL + Q Exits the application.
		info_printf( "Requesting Shutdown through hotkey (CTRL-Q). Calling ShutdownApplication()..." );
		ShutdownApplication();
		return 0;
	}
	int32_t result = 0;
	IGGame* pGame = GetGame();
	if( pGame )
	{
		result = pGame->ProcessUserInput(fElapsedTime);
		if( result )
		{
			error_printf( "IGGame::ProcessUserInput() returned 0x%X", result );
		}
	}

	return 0;
};

//------------------------------------------------------------
// Update game stuff.
int32_t CGApplication::UpdateApplication( float fElapsedTime )
{
	GPROFILE_METHOD( "CGApplication", "UpdateGame" );
	int32_t result = 0;
	// Get the game object and call the function
	IGGame* pGame = GetGame();
	if( pGame )
	{
		result = pGame->Update(fElapsedTime);
		if( result )
		{
			error_printf( "IGGame::Update() returned 0x%X", result );
		}
	}
	return result;
};

//------------------------------------------------------------
// Draw everything.
int32_t CGApplication::DrawWindow( uint32_t nIndex, float fElapsedTime )
{
	GPROFILE_METHOD( "CGApplication", "DrawWindow" );
	int32_t result = 0;
	// Get the game object and call the function
	IGGame* pGame = GetGame();
	if( pGame )
	{
		result = pGame->Draw(fElapsedTime);
		if( result )
		{
			error_printf( "IGGame::Draw() returned 0x%X", result );
		}
	}
	return 0;
};

//------------------------------------------------------------
// Play sounds.
int32_t CGApplication::PlaySounds( float fElapsedTime )
{
	GPROFILE_METHOD( "CGApplication", "PlaySounds" );
	int32_t result = 0;
	IGGame* pGame = GetGame();
	if( pGame )
	{
		result = pGame->PlaySounds(fElapsedTime);
		if( result )
		{
			error_printf( "IGGame::PlaySounds() returned 0x%X", result );
		}
	}
	return 0;
};
//
//----------- End of main loop function calls ----------------

void CGApplication::OnKeyUp( uint8_t key )
{
};

void CGApplication::OnKeyDown( uint8_t key )
{
	GHPTR(Keyboard) pKeyboard = GetKeyboardManager();
	IGGame* pGame = GetGame();
	GHPTR(VideoDevice) pVideo = GetVideoManager();
	GHPTR(Storage) pStorage = GetStorageManager();
	if( !pKeyboard )
	{
		error_printf("GetKeyboardManager() returned NULL! this could mean that other keyboard manager has registered this application as handler.");
		return;
	}
	switch( key )
	{
	case GKEY_F5:	
		//if( pKeyboard->IsKeyDown( GKEY_LCONTROL ) ) 
		//{
		//}
		break; 
	case GKEY_F6:	
		//if( pKeyboard->IsKeyDown( GKEY_LCONTROL ) ) 
		//{
		//}
		break; 
	case GKEY_F7:	
		//if( pKeyboard->IsKeyDown( GKEY_LCONTROL ) ) 
		//{
		//}
		break; 
	case GKEY_F8:	
		//if( pKeyboard->IsKeyDown( GKEY_LCONTROL ) ) 
		//{
		//}
		break; 
	case GKEY_F10:	
		if( pKeyboard->IsKeyDown( GKEY_LCONTROL ) ) 
			genableDebugger( !genabledDebugger() ); 
		break; 
	}		
};
