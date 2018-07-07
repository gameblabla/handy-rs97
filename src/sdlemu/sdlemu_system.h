/*
 * SDLEMU library - free sdl related library functions.
 * Copyrigh(c) 1999-2002 sdlemu development
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

#ifndef SDLEMU_SYSTEM_H
#define SDLEMU_SYSTEM_H

#include <SDL/SDL.h>

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;


typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#ifdef _MSC_VER
typedef unsigned __int64 u64;
#else
typedef unsigned long long u64;
#endif

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#ifdef _MSC_VER
typedef signed __int64 s64;
#else
typedef signed long long s64;
#endif

#ifndef boolean
    #define boolean int
#endif
#ifndef bool
    #define bool    int
#endif

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE  1
#endif


#endif //VBA_SYSTEM_H
