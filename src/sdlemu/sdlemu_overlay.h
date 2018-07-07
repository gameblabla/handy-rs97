/*
 * SDLEMU library - Free sdl related functions library
 * Copyrigh(c) 1999-2002 sdlemu development crew
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __SDLEMU_OVERLAY_H__
#define __SDLEMU_OVERLAY_H__

#include "SDL/SDL.h"
#include "sdlemu_system.h"

SDL_Overlay	    *overlay;
SDL_Rect         overlay_rect;

#ifdef __cplusplus
extern "C" {
#endif

void sdlemu_init_overlay(SDL_Surface *dst, int overlaytype, int width, int height);
void sdlemu_close_overlay(void);
void sdlemu_draw_overlay(SDL_Surface *s, int size, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
