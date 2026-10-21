#include "GDIMouse.h"
#include "GDIKeyboard.h"
#include <list>
#include "IGPrivate.h"

#ifndef _NDINPUT_H
#define _NDINPUT_H

using namespace god;

class CGDInput : public IGInput
{
public:
	CGDInput( void );
	virtual ~CGDInput( void );

	error_t						Init( void );
	error_t						Update( float fElapsedTime );
	IDirectInput8W*				GetDirectInput( void )
	{
		return m_pDI;
	};

	// ---
	error_t						CreateKeyboardManager( IGTrueKeyboardHandler** ppCreated, HWND hDeviceWindow );
	error_t						CreateMouseManager( IGTrueMouseHandler** ppCreated, HWND hDeviceWindow );

protected:

	// DirectInput variables
	IDirectInput8W*					m_pDI;			// The DirectInput context object
	gelementary_old<CGDIKeyboard*>	m_lstKeyboard;
	gelementary_old<CGDIMouse*>		m_lstMouse;

	void							_Cleanup( void );
};

#endif // _NDINPUT_H