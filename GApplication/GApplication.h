/// This file is part of the Game Oriented Development Software Kit
/// Copyright © 2011: Pablo Ariel Zorrilla Cepeda
#include "GFramework.h"

#ifndef _GAPPLICATION_H
#define _GAPPLICATION_H

//using namespace god; // uncomment this if you want and your code allows it
class CGApplication : public god::CGFrameworkWin32
{
public:
	/* ... Declare here your public class members ... */
public:
	CGApplication( void );
	virtual ~CGApplication( void );

	// make some initial configuration for the GUI and everything
	// else you may need at start:
	virtual int32_t			InitApplication( void );
	
	// Process windows messages
	virtual int32_t			ApplicationMsgProc( HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam );
	
	//----------- Main loop function calls --------------
	// Read and process user input.
	virtual int32_t			ProcessUserInput( float fElapsedTime );
	
	// Update game stuff.
	virtual int32_t			UpdateApplication( float fElapsedTime );
	
	// Draw everything.
	virtual int32_t			DrawWindow( uint32_t nWindowIndex, float fElapsedTime );
	
	// Play sounds.
	virtual int32_t			PlaySounds( float fElapsedTime );
	//----------- End of main loop function calls --------------

	// ShutdownApplication may be called either by the user or the runtime, ie. when the user tries to close the window.
	virtual	int32_t			ShutdownApplication( void );

	// The following functions must be defined because CGFramework implements the IGKeyboardHandler interface.
	virtual void			OnKeyUp( uint8_t key );
	virtual void			OnKeyDown( uint8_t key );

private:
	/* ... Declare here your private class members ... */
};

#endif // _GAPPLICATION_H