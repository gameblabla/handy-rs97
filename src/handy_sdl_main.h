//
// Copyright (c) 2004 SDLemu Team
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy/SDL - An Atari Lynx Emulator                 //
//                             Copyright (c) 2005                           //
//                                SDLemu Team                               //
//                                                                          //
//                          Based upon Handy v0.95 WIN32                    //
//                            Copyright (c) 1996,1997                       //
//                                  K. Wilkins                              //
//////////////////////////////////////////////////////////////////////////////
// handy_sdl_main.h                                                         //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the main Handy/SDL header file. It manages the main functions    //
// for emulating the Atari Lynx emulator using the SDL Library.             //
//                                                                          //
//    N. Wagenaar                                                           //
// December 2005                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// December 2005 :                                                          //
//  Since the 14th of April, the WIN32 of Handy (written by Keith Wilkins)  //
//  Handy has become OpenSource. Handy/SDL v0.82 R1 was based upon the old  //
//  v0.82 sources and was released closed source.                           //
//                                                                          //
//  Because of this event, the new Handy/SDL will be released as OpenSource //
//  but is rewritten from scratch because of lost sources (tm). The SDLemu  //
//  team has tried to bring Handy/SDL v0.1 with al the functions from the   //
//  closed source version.                                                  //
//////////////////////////////////////////////////////////////////////////////

#ifndef __HANDY_SDL_MAIN_H__
#define __HANDY_SDL_MAIN_H__

#include <SDL/SDL.h>
#include "system.h"
#include "pixblend.h"
#include "errorhandler.h"


#define		HANDY_SDL_VERSION	"v0.5"
#define     HANDY_VERSION           	"Handy 0.95 WIN32"
#define     HANDY_BASE_FPS              60

/* SDL declarations */
extern SDL_Surface		*HandyBuffer; 			// Our Handy/SDL display buffer
extern SDL_Surface		*mainSurface;	 		// Our Handy/SDL primary display
extern SDL_Surface		*ScreenSurface;	 		// Our Handy/SDL primary display

/* Handy declarations */
extern ULONG			*mpLynxBuffer;
extern CSystem 		    *mpLynx;
extern int				 mFrameSkip;
extern int				 mpBpp;

/* Handy/SDL declarations */
extern int			 	LynxWidth;				// Lynx SDL screen width
extern int			 	LynxHeight;      		// Lynx SDL screen height
extern int 		 	    LynxFormat;				// Lynx ROM format type
extern int 		 	    LynxRotate;				// Lynx ROM rotation type
extern int				LynxScale;				// Scale output
extern int				LynxLCD;                // LCD/Scanline Output
extern int				emulation;              // Emulation enabled
extern int				rendertype;             // SDL Rendertype
extern int				stype;					// Scaling/Scanline Routine
extern int              filter;					// Output Filter
extern uint32_t           overlay_format;         // YUV Overlay format
extern Uint8  		   *delta;

inline	int 	handy_sdl_update(void);
		void 	handy_sdl_rom_info(void);
		int 	main(int argc, char *argv[]);
		void	handy_sdl_quit(void);
		void	handy_sdl_core_reinit(char *romname);
#endif
