#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>

#include "sdlemu/sdlemu_filter.h"
#include "gui/gui.h"

extern SDL_Surface *mainSurface, *HandyBuffer;
extern int mRotation, LynxWidth, LynxHeight;
extern int gui_ImageScaling;
uint32_t Cut_Off_Y = 0;

#define LYNX_DOUBLE_WIDTH (LynxWidth*2)
#define LYNX_DOUBLE_HEIGHT (LynxHeight*2)

#define LYNX_SINGLE_WIDTH (LynxWidth)
#define LYNX_SINGLE_HEIGHT (LynxHeight)

void handy_sdl_draw_graphics(void)
{
	uint32_t y, pitch;
	uint16_t *src_draw, *dst_draw;
	Uint8 *dst_offset;
	SDL_Rect dst, dst2;
	
	if (SDL_LockSurface(mainSurface) == 0)
	{
		/* Horizontal games */
		if (mRotation == 0)
		{
			switch(gui_ImageScaling)
			{
				/* Prefer Keeping Aspect Ratio */
				case 0:
					// (18 * 160) is the Y offset.
					upscale_160x102_to_320xXXX_noAveraging((uint32_t* __restrict__)mainSurface->pixels + (18 * 160), (uint32_t* __restrict__)HandyBuffer->pixels, 204);
				break;
				case 1:
				// Linear filtering (fast upscaler)
					upscale_160x102_to_320xXXX((uint32_t* __restrict__)mainSurface->pixels, (uint32_t* __restrict__)HandyBuffer->pixels, 240);
				break;
			}
		}
		/* Vertical mode games */
		else
		{
			switch(gui_ImageScaling)
			{
				case 0:
					bitmap_scale(0,Cut_Off_Y,
					LynxWidth,LynxHeight,
					LYNX_DOUBLE_WIDTH,mainSurface->h,
					HandyBuffer->w, mainSurface->w-LYNX_DOUBLE_WIDTH,
					(uint16_t* __restrict__)HandyBuffer->pixels,(uint16_t* __restrict__)mainSurface->pixels+(mainSurface->w-(LYNX_DOUBLE_WIDTH))/2+(mainSurface->h-(mainSurface->h))/2*mainSurface->w);
				break;
				case 1:
					bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, mainSurface->w, mainSurface->h, LynxWidth, 0, HandyBuffer->pixels, mainSurface->pixels);
				break;
			}
		}
	}
	SDL_UnlockSurface(mainSurface);
}
