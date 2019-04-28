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
// handy_sdl_graphics.h                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL graphics header file. It manages the graphics      //
// functions for emulating the Atari Lynx emulator using the SDL Library.   //
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

#ifndef __HANDY_SDL_GRAPHICS_H__
#define __HANDY_SDL_GRAPHICS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>

extern void Clean_Surfaces();

#ifdef __cplusplus
extern "C" {
#endif
extern  void    handy_sdl_draw_graphics(void);
#ifdef __cplusplus
}
#endif

inline	void    handy_sdl_scale(void);
inline  void    handy_sdl_draw_filter(int filtertype, SDL_Surface *src, SDL_Surface *dst, Uint8 *delta);
		int 	handy_sdl_video_setup(int rendertype, int fsaa, int fullscreen, int bpp, int scale, int accel, int sysnc);
		void 	handy_sdl_video_init(int bpp);
		int 	handy_sdl_video_setup_opengl(int fsaa, int accel, int sync);
		int 	handy_sdl_video_setup_sdl(const SDL_VideoInfo *info);
		int 	handy_sdl_video_setup_yuv(void);
		uint8_t  *handy_sdl_display_callback(ULONG objref);
		void 	handy_sdl_render_buffer(void);
		void    handy_sdl_video_close(void);
		int		handy_sdl_video_early_setup(int surfacewidth, int surfaceheight, int sdl_bpp_flag, int videoflags);
#endif
