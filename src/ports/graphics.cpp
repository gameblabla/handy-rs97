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
//                          Based upon Handy v0.90 WIN32                    //
//                            Copyright (c) 1996,1997                       //
//                                  K. Wilkins                              //
//////////////////////////////////////////////////////////////////////////////
// handy_sdl_graphics.cpp                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL graphics. It manages the graphics functions        //
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>

#include "handy_sdl_main.h"
#include "handy_sdl_graphics.h"
#include "sdlemu/sdlemu_video.h"
#include "sdlemu/sdlemu_filter.h"
#include "gui/gui.h"

extern SDL_Joystick* joystick;
extern SDL_Surface* menuSurface;


/*
    Name                :     handy_sdl_video_init
    Parameters          :     N/A
    Function            :   Initialisation of Handy/SDL graphics.

    Uses                :   mpLynxBuffer ( Handy core rendering buffer )
                            mpLynx Class ( Handy core )
                            HandyBuffer  ( Handy/SDL display buffer )

    Information            :    Creates the backbuffer for the Handy core based
                            upon rotation, format, etc.
*/
uint8_t *handy_sdl_display_fake_callback(uint32_t objref);
void handy_sdl_video_init(int bpp)
{

    printf("Initialising Handy Display... ");
    switch(bpp)
    {
        case 8:
            LynxFormat = MIKIE_PIXEL_FORMAT_8BPP;
            break;
        case 15:
            LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_555;
            break;
        case 16:
            LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_565;
            break;
        case 24:
            LynxFormat = MIKIE_PIXEL_FORMAT_24BPP;
            break;
        case 32:
            LynxFormat = MIKIE_PIXEL_FORMAT_32BPP;
            break;
        default:
            LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_565; // Default 16BPP bpp
            break;
    }

    // HandyBuffer must be initialized previously
    mpLynxBuffer = (ULONG *)HandyBuffer->pixels;
    mpLynx->DisplaySetAttributes( LynxRotate, LynxFormat, (uint32_t)HandyBuffer->pitch, handy_sdl_display_callback, (ULONG)mpLynxBuffer);

    printf("[DONE]\n");
}

#define SDL_FLAGS SDL_HWSURFACE

/*
    This is called also from gui when initializing for rom browser
*/
int Handy_Init_Video()
{
	if (mainSurface) return 1;
	mainSurface = SDL_SetVideoMode(SDL_OUTPUT_WIDTH, SDL_OUTPUT_HEIGHT, 16, SDL_FLAGS);
	
    if (mainSurface == NULL)
    {
        printf("Could not create primary SDL surface: %s\n", SDL_GetError());
        return 0;
    }
    
    if (menuSurface == NULL)
	{
		menuSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, MENU_OUTPUT_WIDTH, MENU_OUTPUT_HEIGHT, SDL_BPP, 0, 0, 0, 0);
		if (menuSurface == NULL)
		{
			printf("Could not create primary SDL surface: %s\n", SDL_GetError());
			return 0;
		}
	}
    
    SDL_EnableKeyRepeat( 0, 0); // Best options to use
    SDL_ShowCursor( 0 ); // Removing mouse from window. Very handy in fullscreen mode :)
	
	handy_sdl_video_init(SDL_BPP);

    return 1;
}

/*
    This is called also from gui when initializing for rom browser
*/
#ifdef IPU_SCALE
void Handy_Change_Res(uint8_t menu)
{
	#ifdef IPU_SCALE
	if (menu == 1)
	{
		mainSurface = SDL_SetVideoMode(SDL_OUTPUT_WIDTH, SDL_OUTPUT_HEIGHT, 16, SDL_FLAGS);
	}
	else
	{
		if (mRotation == CART_NO_ROTATE) mainSurface = SDL_SetVideoMode(160, 102, 16, SDL_FLAGS);
		else mainSurface = SDL_SetVideoMode(102, 160, 16, SDL_FLAGS);
	}
	#endif
}
#endif

/*
    Name                :     handy_sdl_display_callback
    Parameters          :     N/A
    Function            :   Handy/SDL display rendering function.

    Uses                :   HandyBuffer  ( Handy/SDL display buffer )
                            mainSurface     ( Handy/SDL primary display )

    Information            :    Renders the graphics from HandyBuffer to
                            the main surface.
*/
uint8_t *handy_sdl_display_fake_callback(uint32_t objref)
{
    return (uint8_t *)mpLynxBuffer;
}

uint8_t *handy_sdl_display_callback(ULONG objref)
{
    // Time to render the contents of mLynxBuffer to the SDL gfxBuffer.
    // Now to blit the contents of gfxBuffer to our main SDL surface.
    handy_sdl_draw_graphics();
    
    SDL_Flip( mainSurface );
    
    return (uint8_t *)mpLynxBuffer;
}
