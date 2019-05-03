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

extern SDL_Surface *Scanlines_surface[3], *mainSurface, *HandyBuffer;
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
				case 0:
					dst.x = 0;
					dst.h = ((mainSurface->w / LynxWidth) * LynxHeight);
					dst.y = (mainSurface->h - dst.h) / 2;
					dst.w = mainSurface->w;
					dst2.x = 0;
					dst2.y = 0;
					dst2.w = LynxWidth;
					dst2.h = LynxHeight;
					SDL_SoftStretch(HandyBuffer, &dst2, mainSurface, &dst);
				break;
				case 1:
					dst.x = 0;
					dst.y = 0;
					dst.w = mainSurface->w;
					dst.h = mainSurface->h;
					dst2.x = 0;
					dst2.y = 0;
					dst2.w = LynxWidth;
					dst2.h = LynxHeight;
					SDL_SoftStretch(HandyBuffer, &dst2, mainSurface, &dst);
				break;
			}
		}
		/* Vertical mode games */
		else
		{
			switch(gui_ImageScaling)
			{
				case 0:
					/* This often ends up being too small, double width */
					dst.w = ((mainSurface->h / LynxHeight) * LynxWidth) * 2;
					/* If it ends up being bigger than the screen width then don't double */
					if (dst.w > mainSurface->w) dst.w = ((mainSurface->h / LynxHeight) * LynxWidth);
						
					dst.x = (mainSurface->w - dst.w)/2;
					dst.y = 0;
					dst.h = mainSurface->h;
					dst2.x = 0;
					dst2.y = Cut_Off_Y;
					dst2.w = LynxWidth;
					dst2.h = LynxHeight;
					SDL_SoftStretch(HandyBuffer, &dst2, mainSurface, &dst);
				break;
				case 1:
					dst.y = 0;
					dst.h = mainSurface->h;
					dst.x = 0;
					dst.w = mainSurface->w;
					dst2.x = 0;
					dst2.y = Cut_Off_Y;
					dst2.w = LynxWidth;
					dst2.h = LynxHeight;
					SDL_SoftStretch(HandyBuffer, &dst2, mainSurface, &dst);
				break;
			}
		}
	}
	SDL_UnlockSurface(mainSurface);
}
