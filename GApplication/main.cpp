/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2013: Pablo Ariel Zorrilla Cepeda
#include "GApplication.h"
#include "GCore_printf.h"
#include "GDebugger_core.h"

#include <crtdbg.h>

int WINAPI WinMain ( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	//god::genableDebugger( true );
	GPROFILE_FUNCTION();
	CGApplication Application;

	wchar_t wszGameDll[1024] = {};
	if( lpCmdLine )
	{
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpCmdLine, -1, wszGameDll, 1024 );
		wcscat_s( wszGameDll, L".dll" );
	}	
	else
	{
		wcscpy_s( wszGameDll, L"GGame.dll" );
	}
	if( Application.Init( hInstance, L"GApplication v0.00", wszGameDll, L"GDInput.dll", L"GD3D9.dll", L"GWinSock.dll", 0, L"GStorageWin32.dll", 0, 800, 600 ) )
	{
		error_printf("Failed to initialize application.");
		return EXIT_FAILURE;
	}
	else
	{
		info_printf("Application initialization succeeded.");
	}
	if( Application.Run() )
	{
		error_printf("Application loop returned an error.");
		return EXIT_FAILURE;
	}
	else
	{
		info_printf("Application loop returned without error.");
	}
	info_printf("Exiting... Expect Cleanup!");
	return EXIT_SUCCESS;
};

int main( int argc, char** argv )
{
// Enable run-time memory check for debug builds.
#if defined( DEBUG ) | defined( _DEBUG ) // Choose between either debug or release build
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	//god::genableDebugger( true );
	GPROFILE_FUNCTION();
	CGApplication Application;
	wchar_t wszGameDll[1024] = {};
	//_CrtSetBreakAlloc( 75 );
	if( argc > 1 )
	{
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, argv[1], -1, wszGameDll, 1024 );
		wcscat_s( wszGameDll, 1024, L".dll" );
	}	
	else
	{
		wcscpy_s( wszGameDll, 1024, L"GGame.dll" );
	}

	if( Application.Init( GetModuleHandle(NULL), L"GApplication v0.00", wszGameDll, L"GDInput.dll", L"GD3D9.dll", L"GWinSock.dll", 0, L"GStorageWin32.dll", 0, 1366, 600 ) )
	{
		error_printf("Failed to initialize application.");
		return EXIT_FAILURE;
	}
	else
	{
		info_printf("Application initialization succeeded.");
	}
	if( Application.Run() )
	{
		error_printf("Application loop returned an ERROR!");
		return EXIT_FAILURE;
	}
	else
	{
		info_printf("Application loop returned without error.");
	}
	info_printf("Exiting... Expect Cleanup!");
	return EXIT_SUCCESS;
};