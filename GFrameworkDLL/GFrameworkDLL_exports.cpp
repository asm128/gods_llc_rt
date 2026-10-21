// GFrameworkDLL.cpp : Defines the exported functions for the DLL application.
//

//#include "stdafx.h"
#include "GFrameworkDLL.h"
#include "GApplication.h"
//#include "GLibraryLoader.h"
#include <vector>

//#include "GCore_printf.h"
#include "GDebugger_core.h"
#include <strsafe.h>


struct SFrameworkHWND
{
	HWND				hWnd;
	god::CGFramework*	pFramework;
};

std::vector<SFrameworkHWND> g_lstFrameworks;

GFRAMEWORKDLL_API int gcreateFramework(HWND hWnd)
{
// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	for( uint32_t i=0; i<g_lstFrameworks.size(); i++ )
		if( hWnd == g_lstFrameworks[i].hWnd )
		{
			error_printf("Framework already created for this window.");
			return -1;
		}
	for( uint32_t i=0; i<g_lstFrameworks.size(); i++ )
	{
		SFrameworkHWND* p = &g_lstFrameworks[i];
		if( 0 == p->hWnd && 0 == p->pFramework )
		{
			p->hWnd = hWnd;
			p->pFramework = new god::CGFramework();
			return i;
		}
	}
	SFrameworkHWND newFramework = { hWnd, new god::CGFramework() };
	g_lstFrameworks.push_back(newFramework);
	return g_lstFrameworks.size()-1;
};

GFRAMEWORKDLL_API int ginitFramework(god::id_t frameworkId)
{
	SFrameworkHWND* p = &g_lstFrameworks[frameworkId];
	return p->pFramework->Init(p->hWnd);
};

GFRAMEWORKDLL_API int grunFramework(god::id_t frameworkId)
{
	SFrameworkHWND* p = &g_lstFrameworks[frameworkId];
	return p->pFramework->Run();
};

GFRAMEWORKDLL_API int gshutdownFramework(god::id_t frameworkId)
{
	SFrameworkHWND* p = &g_lstFrameworks[frameworkId];
	return p->pFramework->Run();
};

GFRAMEWORKDLL_API int gdestroyFramework(god::id_t frameworkId)
{
	SFrameworkHWND* p = &g_lstFrameworks[frameworkId];
	if( 0 == p->pFramework )
		return -1;
	god::CGFramework* pFramework = p->pFramework;
	p->pFramework	= 0;
	p->hWnd			= NULL;

	delete( pFramework );
	return -1;
};

GFRAMEWORKDLL_API int gmouseMove		( god::id_t fId, int32_t x, int32_t y, int32_t z )		{ god::CGFramework* pFramework = g_lstFrameworks[fId].pFramework;	if( pFramework ) { pFramework->OnMouseMove( x, y, z );				return 0; } else { error_printf("Invalid framework instance"); return -1; } };
GFRAMEWORKDLL_API int gmouseButtonUp	( god::id_t fId, uint8_t ButtonIndex )					{ god::CGFramework* pFramework = g_lstFrameworks[fId].pFramework;	if( pFramework ) { pFramework->OnMouseButtonUp( ButtonIndex );		return 0; } else { error_printf("Invalid framework instance"); return -1; } };
GFRAMEWORKDLL_API int gmouseButtonDown	( god::id_t fId, uint8_t ButtonIndex )					{ god::CGFramework* pFramework = g_lstFrameworks[fId].pFramework;	if( pFramework ) { pFramework->OnMouseButtonDown( ButtonIndex );	return 0; } else { error_printf("Invalid framework instance"); return -1; } };
GFRAMEWORKDLL_API int gkeyUp			( god::id_t fId, god::GSCANCODE_TYPE Key )				{ god::CGFramework* pFramework = g_lstFrameworks[fId].pFramework;	if( pFramework ) { pFramework->OnKeyUp( Key );						return 0; } else { error_printf("Invalid framework instance"); return -1; } };
GFRAMEWORKDLL_API int gkeyDown			( god::id_t fId, god::GSCANCODE_TYPE Key )				{ god::CGFramework* pFramework = g_lstFrameworks[fId].pFramework;	if( pFramework ) { pFramework->OnKeyDown( Key );					return 0; } else { error_printf("Invalid framework instance"); return -1; } };

// This is an example of an exported variable
GFRAMEWORKDLL_API int nGFrameworkDLL=0;

extern CGApplication* g_Application = 0;

// This is an example of an exported function.
GFRAMEWORKDLL_API int create3DViewport(HWND hWnd)
{
// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	//god::genableDebugger( true );
	GPROFILE_FUNCTION();
	CGApplication Application;
	g_Application = &Application;

	wchar_t wszGameDll[1024] = {};
	//if( lpCmdLine )
	//{
	//	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpCmdLine, -1, wszGameDll, 1024 );
	//	wcscat_s( wszGameDll, L".dll" );
	//}	
	//else
	//{
		wcscpy_s( wszGameDll, L"GShopEdit.dll" );
	//}
	if( Application.Init( hWnd, L"GApplication v0.00", wszGameDll, L"GDInput.dll", L"GD3D9.dll", L"GWinSock.dll", 0, L"GStorageWin32.dll", 0 ) )
	{
		error_printf("Failed to initialize application.");
		return -1;
	}
	else
	{
		info_printf("Application initialization succeeded.");
	}
	if( Application.Run() )
	{
		error_printf("Application loop returned an error.");
		g_Application = 0;
		return -1;
	}
	else
	{
		info_printf("Application loop returned without error.");
		g_Application = 0;
	}
	info_printf("Exiting... Expect Cleanup!");
	return 0;
}

GFRAMEWORKDLL_API god::IGFramework* getFramework(int hWnd)
{
	return g_Application;
};
