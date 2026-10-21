/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2009: Pablo Ariel Zorrilla Cepeda
//
//
//

#include "IGPPU.h"
#include "IGVideo.h"
#include "IGInput.h"
#include "IGAudio.h"
#include "IGNetwork.h"
#include "IGPrivate.h"
#include "IGGame.h"
#include <windows.h>

#ifndef _GDRIVERSLOADER_H
#define _GDRIVERSLOADER_H

namespace god
{

	class CGLibraryLoader
	{
	public:
		CGLibraryLoader( void );
		virtual ~CGLibraryLoader( void );

		int32_t		CreateVideoContext( const wchar_t* wszDllName, IGVideo** ppContext );
		int32_t		DestroyVideoContext( IGVideo* pContext );
		int32_t		CreateInputContext( const wchar_t* wszDllName, IGInput** ppContext );
		int32_t		DestroyInputContext( IGInput* pContext );
		int32_t		CreateAudioContext( const wchar_t* wszDllName, IGAudio** ppContext );
		int32_t		DestroyAudioContext( IGAudio* pContext );
		int32_t		CreateNetworkContext( const wchar_t* wszDllName, IGNetwork** ppContext );
		int32_t		DestroyNetworkContext( IGNetwork* pContext );
		int32_t		CreatePPUContext( const wchar_t* wszDllName, IGPPU** ppContext );
		int32_t		DestroyPPUContext( IGPPU* pContext );
		int32_t		CreateGameContext( const wchar_t* wszDllName, IGGame** ppContext );
		int32_t		DestroyGameContext( IGGame* pContext );
		int32_t		CreateStorage( const wchar_t* wszDllName, GOSH(Storage)* ppContext );

	private:
		// Windows related
		HMODULE						m_hVideoDLL;
		HMODULE						m_hAudioDLL;
		HMODULE						m_hNetworkDLL;
		HMODULE						m_hPPUDLL;
		HMODULE						m_hGameDLL;
		HMODULE						m_hInputDLL;
		HMODULE						m_hStorageDLL;

		// Store the pointer to exported functions
		CREATEGVIDEO				_pCreateGVideo;
		RELEASEGVIDEO				_pReleaseGVideo;

		CREATEGINPUT				_pCreateGInput;
		RELEASEGINPUT				_pReleaseGInput;

		CREATEGAUDIO				_pCreateGAudio;
		RELEASEGAUDIO				_pReleaseGAudio;

		CREATEGNETWORK				_pCreateGNetwork;
		RELEASEGNETWORK				_pReleaseGNetwork;

		CREATEGPARALLELC			_pCreateGPPU;
		RELEASEGPARALLELC			_pReleaseGPPU;

		CREATEGGAME					_pCreateGGame;
		RELEASEGGAME				_pReleaseGGame;

		GCREATEINTERFACE_STORAGE	_pCreateStorage;
		GFREEINTERFACE_STORAGE		_pFreeStorage;
		
		// Store a pointer to the exported object
		IGVideo*					m_pVideoContext;
		IGAudio*					m_pAudioContext;
		IGInput*					m_pInputContext;
		IGNetwork*					m_pNetworkContext;
		IGPPU*				m_pPPUContext;
		IGGame*						m_pGameContext;
	};


};

#endif _GDRIVERSLOADER_H