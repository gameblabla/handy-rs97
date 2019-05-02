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
	Uint8 *dst_offset;
	SDL_Rect dst, dst2;
	if(SDL_MUSTLOCK(mainSurface)) SDL_LockSurface(mainSurface);

	/* Horizontal games */
	if (mRotation == 0)
	{
			switch(mainSurface->w) 
			{
				case 240:
					switch(gui_ImageScaling)
					{
						case 0:
							bitmap_scale(0,Cut_Off_Y,
							LynxWidth,LynxHeight,
							LynxWidth,LynxHeight,
							HandyBuffer->w, mainSurface->w-LynxWidth,
							(uint16_t* __restrict__)HandyBuffer->pixels,(uint16_t* __restrict__)mainSurface->pixels+(mainSurface->w-(LynxWidth))/2+(mainSurface->h-(LynxHeight))/2*mainSurface->w);
						break;
						case 1:
							bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, mainSurface->w, mainSurface->h, LynxWidth, 0, HandyBuffer->pixels, mainSurface->pixels);
						break;
					}
				break;
				case 320:
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
						case 2:
							// Moves screen surface to the right screen position.
							dst.x = 0;
							dst.y = (mainSurface->h - LynxHeight*2) / 2;
							dst.w = LynxWidth*2;
							dst.h = LynxHeight*2;
							
							// This avoids overdraw
							dst2.x = 0;
							dst2.y = 0;
							dst2.w = LynxWidth*2;
							dst2.h = LynxHeight*2;
							// (18 * 160) is the Y offset.
							upscale_160x102_to_320xXXX_noAveraging((uint32_t* __restrict__)mainSurface->pixels + (18 * 160), (uint32_t* __restrict__)HandyBuffer->pixels, 204);
							
							SDL_BlitSurface(Scanlines_surface[0], &dst2, mainSurface, &dst);
							SDL_BlitSurface(Scanlines_surface[2], &dst2, mainSurface, &dst);
						break;
						case 3:
							// This avoids overdraw
							dst2.x = 0;
							dst2.y = 0;
							dst2.w = mainSurface->w;
							dst2.h = mainSurface->h;
							upscale_160x102_to_320xXXX_noAveraging((uint32_t* __restrict__)mainSurface->pixels, (uint32_t* __restrict__)HandyBuffer->pixels, 240);
							
							SDL_BlitSurface(Scanlines_surface[0], &dst2, mainSurface, NULL);
							SDL_BlitSurface(Scanlines_surface[2], &dst2, mainSurface, NULL);
						break;
					}
				break;
				default:
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
						default:
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

				break;
			}
	}
	/* Vertical mode games */
	else
	{
		switch(mainSurface->w) 
		{
			case 240:
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
			break;
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
					dst.w = LynxWidth*2;
					dst.h = mainSurface->h;
							
					dst2.x = 0;
					dst2.y = 0;
					dst2.w = LynxWidth*2;
					dst2.h = mainSurface->h;
				
					bitmap_scale(0,Cut_Off_Y,
					(LYNX_SINGLE_WIDTH),LYNX_SINGLE_HEIGHT,
					(LYNX_DOUBLE_WIDTH),mainSurface->h,
					LynxWidth, mainSurface->w-((LYNX_DOUBLE_WIDTH)),
					(uint16_t* __restrict__)HandyBuffer->pixels,
					(uint16_t* __restrict__)mainSurface->pixels+(mainSurface->w-(LYNX_DOUBLE_WIDTH))/2+(mainSurface->h-(mainSurface->h))/2*mainSurface->w);
					SDL_BlitSurface(Scanlines_surface[1], &dst2, mainSurface, &dst);
					SDL_BlitSurface(Scanlines_surface[2], &dst2, mainSurface, &dst);
				break;
				case 3:
					dst.x = 0;
					dst.y = 0;
					dst.w = mainSurface->w;
					dst.h = mainSurface->h;
					bitmap_scale(0, Cut_Off_Y, LynxWidth, LynxHeight, mainSurface->w, mainSurface->h, HandyBuffer->w, 0, HandyBuffer->pixels, mainSurface->pixels);
					SDL_BlitSurface(Scanlines_surface[1], &dst, mainSurface, NULL);
					SDL_BlitSurface(Scanlines_surface[2], &dst, mainSurface, NULL);
				break;
			}
			break;
			default:
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
					default:
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
			break;
		}
	}

	
	if(SDL_MUSTLOCK(mainSurface)) SDL_UnlockSurface(mainSurface);
}
