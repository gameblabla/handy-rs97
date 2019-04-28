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
#include "scanlines.h"


extern SDL_Surface *HandyBuffer_temp_surface, *Scanlines_surface[3], *mainSurface, *HandyBuffer;
extern int mRotation, LynxWidth, LynxHeight;
extern int gui_ImageScaling;
uint32_t Cut_Off_Y = 0;

#define LYNX_DOUBLE_WIDTH (LynxWidth*2)
#define LYNX_DOUBLE_HEIGHT (LynxHeight*2)

#define LYNX_SINGLE_WIDTH (LynxWidth)
#define LYNX_SINGLE_HEIGHT (LynxHeight)

void handy_sdl_draw_graphics(void)
{
	Uint8 *dst_offset;
	SDL_Rect dst, dst2;
	if(SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);

	if (mRotation == 0)
	{
			switch(mainSurface->w) 
			{
				case 320:
					switch(gui_ImageScaling)
					{
						/* Prefer Keeping Aspect Ratio */
						case 0:
							bitmap_scale(0,0,
							LYNX_SINGLE_WIDTH,LYNX_SINGLE_HEIGHT,
							LYNX_DOUBLE_WIDTH,LYNX_DOUBLE_HEIGHT,
							HandyBuffer->w, mainSurface->w-LYNX_DOUBLE_WIDTH,
							(uint16_t* __restrict__)HandyBuffer->pixels,(uint16_t* __restrict__)mainSurface->pixels+(mainSurface->w-(LYNX_DOUBLE_WIDTH))/2+(mainSurface->h-(LYNX_DOUBLE_HEIGHT))/2*mainSurface->w);
						break;
						case 1:
							bitmap_scale(0, 0, LYNX_SINGLE_WIDTH, LynxHeight, mainSurface->w, mainSurface->h, HandyBuffer->w, 0, HandyBuffer->pixels, mainSurface->pixels);
						break;
						case 2:
							dst.x = 0;
							dst.y = (mainSurface->h - LynxHeight*2) / 2;
							dst2.x = 0;
							dst2.y = 0;
							dst2.w = LynxWidth*2;
							dst2.h = LynxHeight*2;
							bitmap_scale(0, 0, LynxWidth, LynxHeight, LynxWidth*2, LynxHeight*2, HandyBuffer->w, 0, HandyBuffer->pixels, HandyBuffer_temp_surface->pixels);
							SDL_BlitSurface(Scanlines_surface[0], NULL, HandyBuffer_temp_surface, NULL);
							SDL_BlitSurface(Scanlines_surface[2], NULL, HandyBuffer_temp_surface, NULL);
							SDL_BlitSurface(HandyBuffer_temp_surface, &dst2, mainSurface, &dst);
						break;
						case 3:
							bitmap_scale(0, 0, LynxWidth, LynxHeight, LynxWidth*2, LynxHeight*2, HandyBuffer->w, 0, HandyBuffer->pixels, HandyBuffer_temp_surface->pixels);
							SDL_BlitSurface(Scanlines_surface[0], NULL, HandyBuffer_temp_surface, NULL);
							SDL_BlitSurface(Scanlines_surface[2], NULL, HandyBuffer_temp_surface, NULL);
							bitmap_scale(0, 0, (LYNX_DOUBLE_WIDTH), (LYNX_DOUBLE_HEIGHT), mainSurface->w, mainSurface->h, HandyBuffer_temp_surface->w, 0, HandyBuffer_temp_surface->pixels, mainSurface->pixels);
						break;
					}
				break;
				case 400:
					dst.x = 40;
					if (gui_ImageScaling == 0) dst.y = ((mainSurface->h - (LYNX_DOUBLE_HEIGHT))/2);
					else dst.y = 0;
					dst.w = (LYNX_DOUBLE_WIDTH);
					if (gui_ImageScaling == 0) dst.h = (LYNX_DOUBLE_HEIGHT);
					else dst.h = mainSurface->h;
					
					dst2.x = 0;
					dst2.y = 0;
					dst2.w = LynxWidth;
					dst2.h = LynxHeight;
					SDL_SoftStretch(HandyBuffer, &dst2, mainSurface, &dst);
				break;
				case 480:
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
	else
	{
		switch(mainSurface->w) 
		{
			case 320:
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
				case 2:
					dst.x = (mainSurface->w-LYNX_DOUBLE_WIDTH)/2;
					dst.y = 0;
					
					dst2.x = 0;
					dst2.y = 0;
					dst2.w = LynxWidth*2;
					dst2.h = LynxHeight*2;
					
					bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, LynxWidth*2, LynxHeight*2, HandyBuffer->w, 0, HandyBuffer->pixels, HandyBuffer_temp_surface->pixels);
					bitmap_scale(0,0,
					(LYNX_DOUBLE_WIDTH),LYNX_DOUBLE_HEIGHT,
					(LYNX_DOUBLE_WIDTH),mainSurface->h,
					LynxWidth*2, mainSurface->w-((LYNX_DOUBLE_WIDTH)),
					(uint16_t* __restrict__)HandyBuffer_temp_surface->pixels,(uint16_t* __restrict__)mainSurface->pixels+(mainSurface->w-(LYNX_DOUBLE_WIDTH))/2+(mainSurface->h-(mainSurface->h))/2*mainSurface->w);
					SDL_BlitSurface(Scanlines_surface[1], NULL, mainSurface, NULL);
					SDL_BlitSurface(Scanlines_surface[2], &dst2, mainSurface, &dst);
				break;
				case 3:
					dst.x = 0;
					dst.y = 0;
					dst.w = mainSurface->w;
					dst.h = mainSurface->h;
					bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, LynxWidth*2, LynxHeight*2, HandyBuffer->w, 0, HandyBuffer->pixels, HandyBuffer_temp_surface->pixels);
					bitmap_scale(0, 0, LYNX_DOUBLE_WIDTH, LYNX_DOUBLE_HEIGHT, mainSurface->w, mainSurface->h, LynxWidth*2, 0, HandyBuffer_temp_surface->pixels, mainSurface->pixels);
					SDL_BlitSurface(Scanlines_surface[1], &dst, mainSurface, NULL);
					SDL_BlitSurface(Scanlines_surface[2], NULL, mainSurface, NULL);
				break;
			}
			break;
			default:
				dst.y = 0;
				dst.h = mainSurface->h;
				if (gui_ImageScaling == 0)
				{
					dst.x = (mainSurface->w - (LYNX_DOUBLE_WIDTH))/2;
					dst.w = (LYNX_DOUBLE_WIDTH);
				}
				else
				{
					dst.x = 0;
					dst.w = mainSurface->w;
				}
				dst2.x = 0;
				dst2.y = Cut_Off_Y;
				dst2.w = LynxWidth;
				dst2.h = LynxHeight;
				SDL_SoftStretch(HandyBuffer, &dst2, mainSurface, &dst);
			break;
		}
	}

	
	if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface(mainSurface);
}
