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
#include "sdlemu_filter.h"

static Uint32 colorMask = 0xF7DEF7DE;
static Uint32 lowPixelMask = 0x08210821;
static Uint32 qcolorMask = 0xE79CE79C;
static Uint32 qlowpixelMask = 0x18631863;
static Uint32 redblueMask = 0xF81F;
static Uint32 greenMask = 0x7E0;


uint32_t INTERPOLATE (uint32_t A, uint32_t B)
{
	if (A != B)
	{
		return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
		(A & B & lowPixelMask));
	}
	else
	{
		return A;
	}
}

uint32_t Q_INTERPOLATE (uint32_t A, uint32_t B, uint32_t C, uint32_t D)
{
	register uint32_t x = ((A & qcolorMask) >> 2) +
	((B & qcolorMask) >> 2) +
	((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	register uint32_t y = (A & qlowpixelMask) +
	(B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);
	y = (y >> 2) & qlowpixelMask;
	return x + y;
}

void filter_bilinear(uint8_t *srcPtr, uint32_t srcPitch, uint8_t *dstPtr, uint32_t dstPitch, uint32_t width, uint32_t height)
{
	uint32_t nextlineSrc = srcPitch / sizeof(Uint16);
	uint16_t *p = (Uint16 *)srcPtr;
	uint32_t nextlineDst = dstPitch / sizeof(Uint16);
	uint16_t *q = (Uint16 *)dstPtr;

	while(height--) 
	{
		uint32_t i, ii;
		for(i = 0, ii = 0; i < width; ++i, ii += 2) 
		{
			uint16_t A = *(p + i);
			uint16_t B = *(p + i + 1);
			uint16_t C = *(p + i + nextlineSrc);
			uint16_t D = *(p + i + nextlineSrc + 1);
			*(q + ii) = A;
			*(q + ii + 1) = INTERPOLATE(A, B);
			*(q + ii + nextlineDst) = INTERPOLATE(A, C);
			*(q + ii + nextlineDst + 1) = Q_INTERPOLATE(A, B, C, D);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

/* alekmaul's scaler taken from mame4all */
void bitmap_scale(uint32_t startx, uint32_t starty, uint32_t viswidth, uint32_t visheight, uint32_t newwidth, uint32_t newheight,uint32_t pitchsrc,uint32_t pitchdest, uint16_t* __restrict__ src, uint16_t* __restrict__ dst)
{
    uint32_t W,H,ix,iy,x,y;
    x=startx<<16;
    y=starty<<16;
    W=newwidth;
    H=newheight;
    ix=(viswidth<<16)/W;
    iy=(visheight<<16)/H;

    do 
    {
        uint16_t* __restrict__ buffer_mem=&src[(y>>16)*pitchsrc];
        W=newwidth; x=startx<<16;
        do 
        {
            *dst++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        dst+=pitchdest;
        y+=iy;
    } while (--H);
}
