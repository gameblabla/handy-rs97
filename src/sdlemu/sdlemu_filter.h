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

#define RGB32_LOW_BITS_MASK 0x010101

#define BLUE_MASK565 0x001F001F
#define RED_MASK565 0xF800F800
#define GREEN_MASK565 0x07E007E0

#define BLUE_MASK555 0x001F001F
#define RED_MASK555 0x7C007C00
#define GREEN_MASK555 0x03E003E0

#define RGB(r,g,b) ((r)>>3) << systemRedShift | ((g) >> 3) << systemGreenShift | ((b) >> 3) << systemBlueShift

static u8 row_cur[3*322];
static u8 row_next[3*322];
static u8 *rgb_row_cur = row_cur;
static u8 *rgb_row_next = row_next;

static u32 colorMask = 0xF7DEF7DE;
static u32 lowPixelMask = 0x08210821;
static u32 qcolorMask = 0xE79CE79C;
static u32 qlowpixelMask = 0x18631863;
static u32 redblueMask = 0xF81F;
static u32 greenMask = 0x7E0;

static int systemColorDepth  = 16;
static int RGB_LOW_BITS_MASK = 0x821;
extern int systemRedShift;
extern int systemBlueShift;
extern int systemGreenShift;


#ifdef __cplusplus
extern "C" {
#endif

#ifdef MMX
  void _2xSaILine (u8 *srcPtr, u8 *deltaPtr, u32 srcPitch,
                   u32 width, u8 *dstPtr, u32 dstPitch);
  void _2xSaISuperEagleLine (u8 *srcPtr, u8 *deltaPtr,
                             u32 srcPitch, u32 width,
                             u8 *dstPtr, u32 dstPitch);
  void _2xSaISuper2xSaILine (u8 *srcPtr, u8 *deltaPtr,
                             u32 srcPitch, u32 width,
                             u8 *dstPtr, u32 dstPitch);
    void Init_2xSaIMMX (u32 BitFormat);
  void BilinearMMX (u16 * A, u16 * B, u16 * C, u16 * D,
                    u16 * dx, u16 * dy, u8 *dP);
  void BilinearMMXGrid0 (u16 * A, u16 * B, u16 * C, u16 * D,
                         u16 * dx, u16 * dy, u8 *dP);
  void BilinearMMXGrid1 (u16 * A, u16 * B, u16 * C, u16 * D,
                         u16 * dx, u16 * dy, u8 *dP);
  void EndMMX ();
  
  bool cpu_mmx = 1;
#endif
 
#ifdef __cplusplus
}
#endif


#endif