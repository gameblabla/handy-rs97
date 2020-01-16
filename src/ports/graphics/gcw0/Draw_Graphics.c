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
	SDL_Rect dst;
	
	dst.x = 0;
	dst.y = Cut_Off_Y;
	dst.w = LynxWidth;
	dst.h = LynxHeight;
	
	SDL_BlitSurface(HandyBuffer, &dst, mainSurface, NULL);
}
