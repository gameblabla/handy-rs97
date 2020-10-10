#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <SDL/SDL.h>
#include <SDL/SDL_main.h>

#include "sdlemu/sdlemu_filter.h"
#include "shared_config.h"
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
	
	if (SDL_LockSurface(mainSurface) == 0)
	{
		/* Horizontal games */
		if (mRotation == 0)
		{
			switch(gui_ImageScaling)
			{
				case 0:
					pitch = OUTPUT_WIDTH;
					src_draw = (uint16_t* restrict)HandyBuffer->pixels;
					dst_draw = (uint16_t* restrict)mainSurface->pixels
						+ ((OUTPUT_WIDTH - LynxWidth) / 4) * sizeof(uint16_t)
						+ ((OUTPUT_HEIGHT - LynxHeight) / 2) * pitch;
					for (y = 0; y < LynxHeight; y++)
					{
						memmove(dst_draw, src_draw, LynxWidth * sizeof(uint16_t));
						src_draw += LynxWidth;
						dst_draw += pitch;
					}
				break;
				case 1:
					bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, OUTPUT_WIDTH, OUTPUT_HEIGHT, LynxWidth, 0, HandyBuffer->pixels, mainSurface->pixels);
				break;
			}
		}
		/* Vertical mode games */
		else
		{
			switch(gui_ImageScaling)
			{
				case 0:
					pitch = OUTPUT_WIDTH;
					src_draw = (uint16_t* restrict)HandyBuffer->pixels + (Cut_Off_Y * LynxWidth);
					dst_draw = (uint16_t* restrict)mainSurface->pixels
						+ ((OUTPUT_WIDTH - LynxWidth) / 4) * sizeof(uint16_t)
						+ ((OUTPUT_HEIGHT - LynxHeight) / 2) * pitch;
					for (y = 0; y < LynxHeight; y++)
					{
						memmove(dst_draw, src_draw, LynxWidth * sizeof(uint16_t));
						src_draw += LynxWidth;
						dst_draw += pitch;
					}
				break;
				case 1:
					bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, OUTPUT_WIDTH, OUTPUT_HEIGHT, LynxWidth, 0, HandyBuffer->pixels, mainSurface->pixels);
				break;
			}
		}
	}

	SDL_UnlockSurface(mainSurface);
}
