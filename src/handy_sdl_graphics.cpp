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

static SDL_Surface* real_surface;
extern SDL_Joystick* joystick;

extern void upscale_320x240(uint32_t *src, uint32_t *dst, uint32_t height);
extern void upscale_400x240(uint32_t *src, uint32_t *dst, uint32_t height);
extern void upscale_480x272(uint32_t *src, uint32_t *dst, uint32_t height);

/*
    This is called also from gui when initializing for rom browser
*/
int handy_sdl_video_early_setup(int surfacewidth, int surfaceheight, int sdl_bpp_flag, int videoflags)
{
	#ifdef RS97
	surfacewidth = 320;
	surfaceheight = 240;
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
            int                 sdl_bpp_flag;
            int                 surfacewidth;
            int                 surfaceheight;

    // Since we first checked the rotation, based upon that information
    // We setup the width and height of the display.
    //
    // If OpenGL rendering is selected, we choose 640x480 or 480x640 as
    // output. OpenGL rendering can be funky with non-standard resolutions
    // with buggy OpenGL drivers and/or videocards.
    //
    // This is really ugly but good enough for a first version :)
    switch(LynxRotate) {
        case MIKIE_NO_ROTATE:
            LynxWidth  = 160;
            LynxHeight = 102;
#ifndef DINGUX
            if ( rendertype != 2 )
            {
                surfacewidth  = LynxWidth * scale;
                surfaceheight = LynxHeight * scale;
            }
            else
            {
                surfacewidth  = 640;
                surfaceheight = 480;
            }
#endif
            break;
        case MIKIE_ROTATE_L:
        case MIKIE_ROTATE_R:
            LynxWidth  = 102;
            LynxHeight = 160;
#ifndef DINGUX
            if ( rendertype != 2 )
            {
                surfacewidth  = LynxWidth * scale;
                surfaceheight = LynxHeight * scale;
            }
            else
            {
                surfacewidth  = 480;
                surfaceheight = 640;
            }
#endif
            break;
    }

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
#ifdef DINGUX
    videoflags = handy_sdl_video_setup_sdl(info);
#else
    switch(rendertype)
    {
        case 1:
            videoflags = handy_sdl_video_setup_sdl(info);
            break;
        case 2:
            videoflags = handy_sdl_video_setup_opengl(fsaa, accel, sync);
            break;
        case 3:
            videoflags = handy_sdl_video_setup_yuv();
            break;
        default:
            videoflags = handy_sdl_video_setup_sdl(info);
            break;
    }
#endif

    printf("SDL Rendering : %dBPP\n", sdl_bpp_flag);

/*
    Quick fix for handling fullscreen on WIN32. Fullscreen seems to break when using
    hardware surfaces in normal SDL rendering or when using a frame with OpenGL
    rendering. It just goes back in windowed mode or displays it in the window res.
*/
#ifdef DINGUX
    // ignore fullscreen for dingux
#elif defined(HANDY_SDL_WIN32)
    if (fullscreen)
    {
        if ( rendertype != 2 )
            videoflags  = SDL_SWSURFACE | SDL_FULLSCREEN;
        else
            videoflags  = SDL_OPENGL | SDL_NOFRAME | SDL_FULLSCREEN;
    }
#else
    if (fullscreen)
        videoflags |= SDL_FULLSCREEN;
#endif

    // setup SDL video mode
    handy_sdl_video_early_setup(surfacewidth, surfaceheight, sdl_bpp_flag, videoflags);

    // Setup the Handy Graphics Buffer.
    //
    // All the rendering is done in the graphics buffer and is then
    // blitted to the mainSurface and thus to the screen.
    HandyBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
        LynxWidth,
        LynxHeight,
        sdl_bpp_flag,
        0x00000000, 0x00000000, 0x00000000, 0x00000000);
	printf("Width %d, Height %d\n", LynxWidth, LynxHeight);

    if (HandyBuffer == NULL)
    {
        printf("Could not create secondary SDL surface: %s\n", SDL_GetError());
        return 0;
    }

#ifndef DINGUX
    if (rendertype == 2 )
    // Let us setup OpenGL and our rendering texture. We give the src (HandyBuffer) and the
    // dst (mainSurface) display as well as the automatic bpp selection as options so that
    // our texture is automaticly created :)
    {
    sdlemu_init_opengl(HandyBuffer, mainSurface, 1 /*method*/,
            fsaa /*texture type (linear, nearest)*/,
            0 /* Automatic bpp selection based upon src */);
    }

    if (rendertype == 3 )
    {
            sdlemu_init_overlay(mainSurface, overlay_format, LynxWidth , LynxHeight );
    }
#endif

    /* Setting Window Caption */
    SDL_WM_SetCaption( "Handy/SDL", "HANDY");
    SDL_EnableKeyRepeat( 0, 0); // Best options to use
    SDL_EventState( SDL_MOUSEMOTION, SDL_IGNORE); // Ignoring mouse stuff.
    SDL_ShowCursor( 0 ); // Removing mouse from window. Very handy in fullscreen mode :)

    delta = (uint8*)malloc(LynxWidth*LynxHeight*sizeof(uint32_t)*4);
    memset(delta, 255, LynxWidth*LynxHeight*sizeof(uint32_t)*4);

    Init_2xSaI (565);
    systemRedShift   = sdlCalculateShift(HandyBuffer->format->Rmask);
    systemGreenShift = sdlCalculateShift(HandyBuffer->format->Gmask);
    systemBlueShift  = sdlCalculateShift(HandyBuffer->format->Bmask);


    return 1;
}

/*
    Name                :     handy_sdl_video_setup_opengl
    Parameters          :     fsaa ( 0 = off, 1 = on ) -> OpenGL specific
    Function            :   Initialisation of OpenGL videodriver.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            OpenGL video setup.
*/
#ifndef DINGUX
int handy_sdl_video_setup_opengl(int fsaa,int accel, int sync)
{
    uint32_t             videoflags;

    printf("OpenGL\n");
    // Initializing SDL attributes with OpenGL
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE  , 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        printf("OpenGL OPTION : Enable  SDL_GL_DOUBLEBUFFER\n");

    // Setup FSAA
    if ( fsaa )
    {
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, fsaa );
        printf("OpenGL FSAA   : Enable  SDL_GL_MULTISAMPLEBUFFERS\n");
    }
    else
    {
        printf("OpenGL FSAA   : Disable SDL_GL_MULTISAMPLEBUFFERS\n");
//        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
        accel = 0;
    }

    if ( accel )
    {
        printf("OpenGL ACCEL  : Enable  SDL_GL_ACCELERATED_VISUAL\n");
        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    }
    else
    {
        printf("OpenGL ACCEL  : Disable SDL_GL_ACCELERATED_VISUAL\n");
//        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 0 );
    }

    if ( sync )
    {
        printf("OpenGL VSYNC  : Enable  SDL_GL_SWAP_CONTROL\n");
        SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
    }
    else
    {
        printf("OpenGL VSYNC  : Disable SDL_GL_SWAP_CONTROL\n");
        SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0 );
    }

    videoflags = SDL_OPENGL;

    return videoflags;

}
#endif

/*
    Name                :     handy_sdl_video_setup_yuv
    Parameters          :     N/A
    Function            :   Initialisation of YUV videodriver.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            YUV video setup.
*/
#ifndef DINGUX
int handy_sdl_video_setup_yuv(void)
{
    uint32_t             videoflags;

    printf("YUV Overlay\n");

    videoflags = SDL_SWSURFACE;
    
    return videoflags;
}
#endif

/*
    Name                :     handy_sdl_video_setup_sdl
    Parameters          :     info (SDL videoinfo)
    Function            :   Initialisation of SDL videodriver.

    Uses                :   N/A

    Information            :    This is our setup function for getting our desired
                            SDL video setup. Using info it automaticly checks
                            if we can use hardware acceleration or going back
                            to software display.
*/
int handy_sdl_video_setup_sdl(const SDL_VideoInfo *info)
{
    uint32_t             videoflags;
#ifdef DINGUX
    videoflags = SDL_HWSURFACE |
    #ifdef SDL_TRIPLEBUF
    SDL_TRIPLEBUF
    #else
    SDL_DOUBLEBUF
    #endif
    ;
#else
    if (info->hw_available)
    {
        printf("SDL Hardware\n");
        videoflags = SDL_HWSURFACE | SDL_HWPALETTE;

        if (info->blit_hw) videoflags |= SDL_HWACCEL;
    }
    else
    {
            printf("SDL Software\n");
            videoflags = SDL_SWSURFACE;
    }
#endif
    return videoflags;
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
    int filter =  1;
    // Time to render the contents of mLynxBuffer to the SDL gfxBuffer.
    // Now to blit the contents of gfxBuffer to our main SDL surface.
    handy_sdl_draw_graphics();
    
    // show fps if needed
    gui_ShowFPS();
    SDL_Flip( mainSurface );
    
    return (uint8_t *)mpLynxBuffer;
}

extern int gui_ImageScaling;

inline void handy_sdl_draw_graphics(void)
{
	Uint8 *dst_offset;
	SDL_Rect dst;
	if(SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);
	
	if (mpLynx->CartGetRotate() == 0)
	{
			switch(mainSurface->w) 
			{
				case 320:
					dst.x = 0;
					if (gui_ImageScaling == 0) dst.y = ((mainSurface->h - (LynxHeight*2))/2);
					else dst.y = 0;
					dst.w = (LynxWidth*2);
					if (gui_ImageScaling == 0) dst.h = (LynxHeight*2);
					else dst.h = mainSurface->h;
					SDL_SoftStretch(HandyBuffer, NULL, mainSurface, &dst);
				break;
				case 400:
					dst.x = 40;
					if (gui_ImageScaling == 0) dst.y = ((mainSurface->h - (LynxHeight*2))/2);
					else dst.y = 0;
					dst.w = (LynxWidth*2);
					if (gui_ImageScaling == 0) dst.h = (LynxHeight*2);
					else dst.h = mainSurface->h;
					SDL_SoftStretch(HandyBuffer, NULL, mainSurface, &dst);
				break;
				case 480:
					dst.x = 0;
					dst.y = 0;
					dst.w = mainSurface->w;
					dst.h = mainSurface->h;
					SDL_SoftStretch(HandyBuffer, NULL, mainSurface, &dst);
				break;
			}
	}
	else
	{
		SDL_Rect dst;
		dst.x = (mainSurface->w - (LynxWidth*2))/2;
		dst.y = 0;
		dst.w = (LynxWidth*2);
		dst.h = mainSurface->h;
		SDL_SoftStretch(HandyBuffer, NULL, mainSurface, &dst);
	}
	
	if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface(mainSurface);
}

#ifndef DINGUX
inline void handy_sdl_scale(void)
{
    Uint8             bpp;
    char *dcp, *scp, *olddcp;
    int   x, y, *sip, *dip, *olddip, i;
    long *dlp, *slp, *olddlp;
    int increment, copysize;

    bpp = mainSurface->format->BytesPerPixel;

    // SLOW !!!
    if (SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);

    increment = LynxScale*(LynxScale-1)*LynxWidth;
    copysize = increment*bpp;

    switch (bpp)
    {
        case 1:
            scp=(char *) HandyBuffer->pixels;
            dcp=(char *) mainSurface->pixels;
            for (y=0; y< LynxHeight; y++)
            {
                olddcp=dcp;
                for (x=0; x<LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dcp=*scp;
                        dcp++;
                    }
                        scp++;
                }
                if (!LynxLCD)
                    memcpy (dcp, olddcp, copysize);

                dcp+=increment;
            }
            break;
        case 2:
            sip=(int *) HandyBuffer->pixels;
            dip=(int *) mainSurface->pixels;
            for (y=0; y<= LynxHeight; y++)
            {
                olddip=dip;
                for (x=0; x<=LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dip=*sip;
                        dip++;
                    }
                    sip++;
                }
                if (!LynxLCD)
                    memcpy (dip, olddip, copysize);

                dip+=increment;
            }
            break;
        case 3:
            // REALLY SLOW
            scp=(char *) HandyBuffer->pixels;
            dcp=(char *) mainSurface->pixels;
            for (y=0; y< LynxHeight; y++)
            {
                olddcp=dcp;
                for (x=0; x<LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dcp=*scp; dcp++;
                        *dcp=*scp; dcp++;
                        *dcp=*scp; dcp++;
                    }
                    scp+=3;
                }
                if (!LynxLCD)
                    memcpy (dcp, olddcp, copysize);

                dcp+=3*increment;
            }
            break;
        case 4:
        default:
            slp=(long *) HandyBuffer->pixels;
            dlp=(long *) mainSurface->pixels;
            for (y=0; y< LynxHeight; y++)
            {
                olddlp=dlp;
                for (x=0; x<LynxWidth; x++)
                {
                    for (i=0; i<LynxScale; i++)
                    {
                        *dlp=*slp;
                        dlp++;
                    }
                    slp++;
                }
                if (!LynxLCD)
                    memcpy (dlp, olddlp, copysize);

                dlp+=increment;
            }
            break;
    }

    if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface (mainSurface);

}
#endif
/*
    Name                :     handy_sdl_video_close
    Parameters          :     N/A
    Function            :   Handy/SDL Video destroy function.

    Uses                :   mpLynxBuffer ( Handy core rendering buffer )
                            HandyBuffer  ( Handy/SDL buffer display )

    Information            :    Renders the graphics from HandyBuffer to
                            the main surface.
*/
#ifndef DINGUX
void    handy_sdl_video_close(void)
{
    sdlemu_close_overlay();
}
#endif
