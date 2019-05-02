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
#ifndef DINGUX
#include "sdlemu/sdlemu_opengl.h"
#include "sdlemu/sdlemu_overlay.h"
#include "sdlemu/sdlemu_video.h"
#endif
#include "sdlemu/sdlemu_filter.h"
#include "gui/gui.h"

SDL_Surface *Scanlines_surface[3];
extern SDL_Joystick* joystick;
int	sdl_bpp_flag;

void Clean_Surfaces()
{
	uint32_t i;
	
	for(i=0;i<3;i++)
	{
		if (Scanlines_surface[i])
		{
			SDL_FreeSurface(Scanlines_surface[i]);
			Scanlines_surface[i] = NULL;
		}
	}	
}

/*
    This is called also from gui when initializing for rom browser
*/
int handy_sdl_video_early_setup(int surfacewidth, int surfaceheight, int sdl_bpp_flag, int videoflags)
{
	#ifdef RS97
	surfacewidth = 320;
	surfaceheight = 240;
	#elif defined(RS90)
	surfacewidth = 240;
	surfaceheight = 160;
	#elif defined(RETROSTONE)
	surfacewidth = 0;
	surfaceheight = 0;
	#else
	surfacewidth = 480;
	surfaceheight = 272;
	#endif
	mainSurface = SDL_SetVideoMode(surfacewidth, surfaceheight, 16, videoflags);
	
    if (mainSurface == NULL)
    {
        printf("Could not create primary SDL surface: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

/*
    Name                :     handy_sdl_video_setup
    Parameters          :     rendertype ( 1 = SDL, 2 = OpenGL, 3 = YUV )
                            fsaa ( 0 = off, 1 = on ) -> OpenGL specific
                            fullscreen ( 0 = off, 1 = on )
    Function            :   Initialisation of the video using the SDL libary.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            video setup.
*/
int handy_sdl_video_setup(int rendertype, int fsaa, int fullscreen, int bpp, int scale, int accel, int sync)
{
    const    SDL_VideoInfo     *info;
	uint32_t             videoflags;
	int              value;
	int                 surfacewidth;
	int                 surfaceheight;
	SDL_Surface* tmp;
	extern void Set_Rotation_Game();

    info = SDL_GetVideoInfo();

    // Let us check if SDL could get information about the videodriver.
    if (!info)
    {
        printf("ERROR: SDL is unable to get the video info: %s\n", SDL_GetError());
        return false;
    }

    if( bpp != 0 )
    {
            sdl_bpp_flag = bpp;
    }
    else
    {
            switch(info->vfmt->BitsPerPixel)
            {
                case 8:
                    sdl_bpp_flag = 8;
                    break;
                case 16:
                    sdl_bpp_flag = 16;
                    break;
                case 24:
                    sdl_bpp_flag = 24;
                    break;
                case 32:
                    sdl_bpp_flag = 32;
                    break;
                default:
                    sdl_bpp_flag = 8;  // Default : 8bpp
                    break;
            }
    }

    mpBpp = sdl_bpp_flag;

    printf("\nSDL Rendering : ");
    videoflags = SDL_HWSURFACE;

    printf("SDL Rendering : %dBPP\n", sdl_bpp_flag);

    // setup SDL video mode
    handy_sdl_video_early_setup(surfacewidth, surfaceheight, sdl_bpp_flag, videoflags);

    // Setup the Handy Graphics Buffer.
    //
    // All the rendering is done in the graphics buffer and is then
    // blitted to the mainSurface and thus to the screen.
	HandyBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, LynxWidth, 168, sdl_bpp_flag, 0x00000000, 0x00000000, 0x00000000, 0x00000000);
	
    /* Scanline specific code - Gameblabla */
    if (!Scanlines_surface[0])
    {
		tmp = SDL_LoadBMP("scanlines.bmp");
		if (tmp)
		{
			SDL_SetColorKey(tmp, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(tmp->format, 255, 0, 255));
			Scanlines_surface[0] = SDL_DisplayFormat(tmp);
			SDL_FreeSurface(tmp);
			SDL_SetAlpha(Scanlines_surface[0], SDL_SRCALPHA, 128);
		}
	}
	
	if (!Scanlines_surface[1])
	{
		tmp = SDL_LoadBMP("scanlines_90.bmp");
		if (tmp)
		{
			SDL_SetColorKey(tmp, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(tmp->format, 255, 0, 255));
			Scanlines_surface[1] = SDL_DisplayFormat(tmp);
			SDL_FreeSurface(tmp);
			SDL_SetAlpha(Scanlines_surface[1], SDL_SRCALPHA, 128);
		}
	}
    
    if (!Scanlines_surface[2])
    {
		Scanlines_surface[2] = SDL_CreateRGBSurface(SDL_SWSURFACE, 320 , 320 , sdl_bpp_flag, 0x00000000, 0x00000000, 0x00000000, 0x00000000);
		SDL_FillRect(Scanlines_surface[2], NULL, SDL_MapRGB(Scanlines_surface[2]->format, 255, 255, 255));
		SDL_SetAlpha(Scanlines_surface[2], SDL_SRCALPHA, 128);
	}
	
    if (HandyBuffer == NULL)
    {
        printf("Could not create secondary SDL surface: %s\n", SDL_GetError());
        return 0;
    }

    /* Setting Window Caption */
    SDL_WM_SetCaption( "Handy/SDL", "HANDY");
    SDL_EnableKeyRepeat( 0, 0); // Best options to use
    SDL_EventState( SDL_MOUSEMOTION, SDL_IGNORE); // Ignoring mouse stuff.
    SDL_ShowCursor( 0 ); // Removing mouse from window. Very handy in fullscreen mode :)
    
    return 1;
}

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
    
    // show fps if needed
    gui_ShowFPS();
    SDL_Flip( mainSurface );
    
    return (uint8_t *)mpLynxBuffer;
}
