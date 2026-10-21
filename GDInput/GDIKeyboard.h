#include "IGInput.h"
#include "IGPrivate.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "garray.h"

#ifndef _GDIKEYBOARD_H
#define _GDIKEYBOARD_H

using namespace god;

class CGDInput;

class CGDIKeyboard : public IGTrueKeyboardHandler//_GOSH(Keyboard)
{
public:
	CGDIKeyboard( CGDInput* pInput );
	virtual ~CGDIKeyboard( void );

	// Initialization function
	virtual int32_t					Init( HWND hWnd );
	virtual int32_t					GetID( void ) const { return m_nID; };
	virtual uint32_t				GetVersion( void )const { return 0; };
	virtual GDRIVERFAMILY_ID		GetFamilyID( void )const { return GFAMILYID_HID; };
	virtual GRDS(SDevice)			GetDeviceData( void )
	{ 
		return gacquire( m_pDriverHALData );
	};
	virtual void*					GetPrivateData( void )
	{
		if( m_pDIK ) 
			m_pDIK->AddRef();
		return m_pDIK;
	}; 	
	// ----------- Inherited from IGKeyboard interface.
	// Main keyboard functions
	virtual bool					IsKeyDown( UCHAR vkKey ) const {return ( m_diks[ vkKey ] & 0x80 ) != 0;};
	virtual void					DisableWindowsKey( bool bDisable );
	virtual int32_t					AddHandler( IGKeyboardHandler* pHandler );
	virtual int32_t					RemoveHandler( IGKeyboardHandler* pHandler );

	// ----------- Inherited from IGInputDevice interface.
	virtual int32_t					Poll( FLOAT fElapsedTime );

	// Process windows messages
	virtual int32_t					MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );
	virtual void					SetImmediate( bool bImmediate );
	virtual void					SetExclusive( bool bExclusive );
	virtual void					SetForeground( bool bForeground );
	
	// --- 
	virtual bool					GetProfilingEnabled( void ) const;
	virtual void					SetProfilingEnabled( bool bEnabled );
	virtual void*					GetInterfacePointer( GINTERFACEID_TYPE InterfaceType );
	virtual GINTERFACESTATE_TYPE	GetInterfaceState( void ) const
	{ return __nRefCount ? GISTATE_NORMAL : GISTATE_UNUSED; };
private:
	int32_t							m_nID;
	gpod_ptr<SDevice>				m_pDriverHALData;
	gelementary_old<IGKeyboardHandler*> m_lstHandler;
	CGDInput*						m_pContext;

	// --- DirectInput related members
	HWND								m_hWnd;			// Main device window
	IDirectInputDevice8W*				m_pDIK;		// The keyboard device 

	// --- Object state members
	bool							m_bAcquired;	// Set to true if device is acquired
	bool							m_bImmediate;	// Set to false to start in buffered mode
	bool							m_bExclusive;	// Set to true if start in exclusive mode
	bool							m_bForeground;	// Set to true to start in foreground mode
	unsigned char					m_diks[256];			// Keyboard state
	unsigned char					m_diksPrev[256];		// Previous keyboard state
	bool							m_bDisableWindowsKey;	// Set to true to disable windows key

	// --- CGDIKeyboard private member functions
	void							_HandleStates( void );

	long							_CreateDevice( void );
	long							_AcquireDirectInput( void );
	long							_UnacquireDirectInput( void );
	long							_SetCooperativeLevel( void );
	void							_Cleanup( void );
};

#endif // _GDIKEYBOARD_H 