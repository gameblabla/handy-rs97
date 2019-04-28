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
#ifndef __SDLEMU_FILTER_H__
#define __SDLEMU_FILTER_H__

#include "sdlemu_system.h"
#include "sdlemu_filter_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void bitmap_scale(uint32_t startx, uint32_t starty, uint32_t viswidth, uint32_t visheight, uint32_t newwidth, uint32_t newheight,uint32_t pitchsrc,uint32_t pitchdest, uint16_t* __restrict__ src, uint16_t* __restrict__ dst);
extern void filter_bilinear(uint8_t *srcPtr, uint32_t srcPitch, uint8_t *dstPtr, uint32_t dstPitch, uint32_t width, uint32_t height);

 
#ifdef __cplusplus
}
#endif


#endif
