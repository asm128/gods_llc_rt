#include "IGInput.h"
#include "IGPrivate.h"
#define DIRECTINPUT_VERSION 0x0800
#include "garray.h"
#include <dinput.h>
#include <list>

#ifndef _GDIMOUSE_H
#define _GDIMOUSE_H

using namespace god;

class CGDInput; 

class CGDIMouse : public IGTrueMouseHandler//_GOSH(Mouse)
{
public:
	CGDIMouse( CGDInput* pInput );
	virtual ~CGDIMouse( void );

	// Initialization function
	virtual int32_t					Init( HWND hWnd );
	virtual int32_t					GetID( void ) const
	{
		return m_nID;
	};
	virtual uint32_t				GetVersion( void ) const
	{
		return 0;
	};
	virtual GDRIVERFAMILY_ID		GetFamilyID( void ) const { return GFAMILYID_HID; };
	virtual GRDS(SDevice)			GetDeviceData( void )
	{ 
		return gacquire( m_pDriverHALData );
	};
	virtual void*					GetPrivateData( void )
	{
		if( 0 == m_pDIM ) 
			return 0;
		m_pDIM->AddRef();
		return m_pDIM;
	};
	// ----------- Inherited from IGMouse interface.
	// Main mouse functions
	virtual int32_t						GetMouseX( void ) const {return m_dims2.lX;};
	virtual int32_t						GetMouseY( void ) const {return m_dims2.lY;};
	virtual int32_t						GetMouseZ( void ) const {return m_dims2.lZ;};
	virtual bool 						IsButtonDown( uint8_t iButton ) const 
	{
		return ( (0 != m_dims2.rgbButtons[iButton])? true : false );
	};
	//
	virtual error_t						AddHandler( IGMouseHandler* pHandler );
	virtual error_t						RemoveHandler( IGMouseHandler* pHandler );
	
	// ----------- Inherited from IGInputDevice interface.
	virtual error_t						Poll( float fElapsedTime );
	virtual void						SetImmediate( bool bImmediate );
	virtual void						SetExclusive( bool bExclusive );
	virtual void						SetForeground( bool bForeground );

	// --- 
	virtual error_t						MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );
	//--
	virtual bool						GetProfilingEnabled( void ) const;
	virtual void						SetProfilingEnabled( bool bEnabled );
	virtual void*						GetInterfacePointer( GINTERFACEID_TYPE InterfaceType );
	virtual GINTERFACESTATE_TYPE		GetInterfaceState( void ) const
	{ return __nRefCount ? GISTATE_NORMAL : GISTATE_UNUSED; };

private:
	int32_t								m_nID;
	gpod_ptr<SDevice>					m_pDriverHALData;
	gelementary_old<IGMouseHandler*>	m_lstHandler;
	CGDInput*							m_pContext;

	// --- DirectInput related members
	IDirectInputDevice8W*				m_pDIM;			// The mouse device 
	HWND								m_hWnd;			// Main device window
	DIMOUSESTATE2						m_dims2;		// DirectInput Mouse state 
	DIMOUSESTATE2						m_dims2Prev;	// Previous DirectInput Mouse state 

	// --- Object state members
	bool								m_bAcquired;	// Set to true if device is acquired
	bool								m_bImmediate;	// Set to false to start in buffered mode
	bool								m_bExclusive;	// Set to true if start in exclusive mode
	bool								m_bForeground;	// Set to true to start in foreground mode
	
	// --- CGDIMouse private member functions
	void								_HandleStates( void );

	long								_CreateDevice( void );
	long								_AcquireDirectInput( void );
	long								_UnacquireDirectInput( void );
	long								_SetCooperativeLevel( void );
	void								_Cleanup( void );
};

#endif // _GDIMOUSE_H