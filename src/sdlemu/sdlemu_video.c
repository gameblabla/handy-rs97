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

#include "sdlemu_video.h"
 
static inline Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;
    }
}

static inline void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

inline void sdlemu_video(SDL_Surface *s, SDL_Surface *d)
{

    SDL_Rect area;

    area.x = 0;
    area.y = 0;
    area.w = s->w;
    area.h = s->h;
    SDL_BlitSurface( s, &area, d, &area);
}

inline void sdlemu_vidstretch_1(SDL_Surface *s, SDL_Surface *d, int width, int height, int size)
{
    register int x=0,y=0, t1 = 0, t2 = 0;
    Uint32 pixel;

    bool scanline = TRUE;


    if( size >= 2 )
    {
      for(y=0;y< (height); y++)
      {
         for(x=0;x<(width);x++)
         {
            pixel = getpixel( s, x, y );
            for( t1 = 0; t1 < size ; t1++)
            {
               for(t2=0; t2 < size ; t2++)
               {   
                  putpixel( d, ((x*size) + t2) , ((y*size) + t1), pixel);
               }
            }
         }
      }
    }
    else 
        /* Normally we shouldn't come here, but just in case */
        sdlemu_video(s, d);
}

inline void sdlemu_vidstretch_2(SDL_Surface *s, SDL_Surface *d, int width, int height, int size)
{
    SDL_Rect src,dest;

    if( size >= 2 ) {
        src.x = 0;
        src.y = 0;
        src.w = width;
        src.h = height;
        dest.x = 0;
        dest.y = 0;
        dest.w = d->w;
        dest.h = d->h;
        SDL_SoftStretch(s, &src, d, &dest);
    } else 
        /* Normally we shouldn't come here, but just in case */
        sdlemu_video(s, d);
   
}

inline void sdlemu_scanline_1(SDL_Surface *s, SDL_Surface *d, int width, int height, int size)
{
    register int x=0,y=0, t1 = 0, t2 = 0;
    int Scanline_core = 0;
    Uint32 pixel;

    for(y=0;y< (height); y++)
    {
         for(x=0;x<(width);x++)
         {
            pixel = getpixel( s, x, y );
            if ( Scanline_core == 1 ) 
                  putpixel( d, (x*size) , ( y*size) , pixel);
            else {
                  for( t2=0; t2<size; t2++)
                       putpixel( d, (x*size) + t2 , ( y*size) , pixel);
                 }
         }
    }

}

inline void sdlemu_scanline_2(SDL_Surface *s, SDL_Surface *d, int width, int height, int size)
{
    SDL_Rect src,dest;
    register int i=0;

    for(i = 0; i < height; i++) {
           src.x = 0;
           src.y = i;
           src.w = width;
           src.h = 1;
           dest.x = 0;
           dest.y = (((d->h/size)-height)+(i *size));
           dest.w = d->w;
           dest.h = 1;
           SDL_SoftStretch(s, &src, d, &dest);
    }

}
