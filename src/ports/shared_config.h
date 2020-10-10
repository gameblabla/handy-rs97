#ifndef GRAPHICS_CONFIG_H
#define GRAPHICS_CONFIG_H

#if defined(RS90)

	#define SDL_OUTPUT_WIDTH 240
	#define SDL_OUTPUT_HEIGHT 160
	
	#define MENU_OUTPUT_WIDTH 240
	#define MENU_OUTPUT_HEIGHT 160
	
	#define OUTPUT_WIDTH 240
	#define OUTPUT_HEIGHT 160
	#define SDL_BPP 16
	
	#define NONBLOCKING_AUDIO
	/* RS-90 supports IPU scaling but in the case of the Atari Lynx, it's not worth the extra performance cost in my opinion. */
	#undef IPU_SCALE
	
#elif defined(RETROSTONE)
	/* Zero means auto-detect */
	#define SDL_OUTPUT_WIDTH 0
	#define SDL_OUTPUT_HEIGHT 0
	
	#define MENU_OUTPUT_WIDTH 320
	#define MENU_OUTPUT_HEIGHT 240
	
	#define OUTPUT_WIDTH mainSurface->w
	#define OUTPUT_HEIGHT mainSurface->h
	#define SDL_BPP 16
	
	#undef NONBLOCKING_AUDIO
	#undef IPU_SCALE
	
#elif defined(ARCADEMINI)
	/* Zero means auto-detect */
	#define SDL_OUTPUT_WIDTH 480
	#define SDL_OUTPUT_HEIGHT 272
	
	#define MENU_OUTPUT_WIDTH 320
	#define MENU_OUTPUT_HEIGHT 240
	
	#define OUTPUT_WIDTH 480
	#define OUTPUT_HEIGHT 272
	#define SDL_BPP 16
	
	#define NONBLOCKING_AUDIO
	#undef IPU_SCALE

#elif defined(GCW0)
	/* Zero means auto-detect */
	#define SDL_OUTPUT_WIDTH 320
	#define SDL_OUTPUT_HEIGHT 240
	
	#define MENU_OUTPUT_WIDTH 320
	#define MENU_OUTPUT_HEIGHT 240
	
	#define OUTPUT_WIDTH 320
	#define OUTPUT_HEIGHT 240
	#define SDL_BPP 16
	
	#define NONBLOCKING_AUDIO
	#define IPU_SCALE
	
#else
	/* Zero means auto-detect */
	#define SDL_OUTPUT_WIDTH 320
	#define SDL_OUTPUT_HEIGHT 240
	
	#define MENU_OUTPUT_WIDTH 320
	#define MENU_OUTPUT_HEIGHT 240
	
	#define OUTPUT_WIDTH 320
	#define OUTPUT_HEIGHT 240
	#define SDL_BPP 16
	
	#undef NONBLOCKING_AUDIO
	#undef IPU_SCALE
#endif

#ifdef NONBLOCKING_AUDIO
	#ifdef SDL_TRIPLEBUF
	#define SDL_FLAGS SDL_HWSURFACE | SDL_TRIPLEBUF
	#else
	#define SDL_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF
	#endif
#else
	#define SDL_FLAGS SDL_HWSURFACE
#endif

#endif
