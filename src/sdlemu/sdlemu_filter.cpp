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
//extern "C" {
//#include "sdlemu_filter_defs.h"
//};

int             systemRedShift    = 0;
int             systemBlueShift   = 0;
int 			systemGreenShift  = 0;

int sdlCalculateShift(u32 mask)
{
  int m = 0;
  
  while(mask) {
    m++;
    mask >>= 1;
  }

  return m-5;
}

/* 2xSAI, SuperSai, etc filter (VisualBoy Advance, Forgotten) */

int Init_2xSaI(u32 BitFormat)
{
  if(systemColorDepth == 16) {
    if (BitFormat == 565) {
      colorMask = 0xF7DEF7DE;
      lowPixelMask = 0x08210821;
      qcolorMask = 0xE79CE79C;
      qlowpixelMask = 0x18631863;
      redblueMask = 0xF81F;
      greenMask = 0x7E0;
    } else if (BitFormat == 555) {
      colorMask = 0x7BDE7BDE;
      lowPixelMask = 0x04210421;
      qcolorMask = 0x739C739C;
      qlowpixelMask = 0x0C630C63;
      redblueMask = 0x7C1F;
      greenMask = 0x3E0;
    } else {
      return 0;
    }
  } else if(systemColorDepth == 32) {
    colorMask = 0xfefefe;
    lowPixelMask = 0x010101;
    qcolorMask = 0xfcfcfc;
    qlowpixelMask = 0x030303;
  } else
    return 0;

#ifdef MMX
    Init_2xSaIMMX (BitFormat);
#endif
  
  return 1;
}

static inline int GetResult1 (u32 A, u32 B, u32 C, u32 D,
                              u32 /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
      x += 1;
    else if (B == C)
      y += 1;
    if (A == D)
      x += 1;
    else if (B == D)
      y += 1;
    if (x <= 1)
      r += 1;
    if (y <= 1)
      r -= 1;
    return r;
}

static inline int GetResult2 (u32 A, u32 B, u32 C, u32 D,
                              u32 /* E */)
{
  int x = 0;
  int y = 0;
  int r = 0;
  
  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r -= 1;
  if (y <= 1)
    r += 1;
  return r;
}

static inline int GetResult (u32 A, u32 B, u32 C, u32 D)
{
  int x = 0;
  int y = 0;
  int r = 0;
  
  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r += 1;
  if (y <= 1)
    r -= 1;
  return r;
}

static inline u32 INTERPOLATE (u32 A, u32 B)
{
  if (A != B) {
    return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
            (A & B & lowPixelMask));
  } else
    return A;
}

static inline u32 Q_INTERPOLATE (u32 A, u32 B, u32 C, u32 D)
{
  register u32 x = ((A & qcolorMask) >> 2) +
    ((B & qcolorMask) >> 2) +
    ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
  register u32 y = (A & qlowpixelMask) +
    (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);
  
  y = (y >> 2) & qlowpixelMask;
  return x + y;
}

static inline int GetResult1_32 (u32 A, u32 B, u32 C, u32 D,
                                 u32 /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
      x += 1;
    else if (B == C)
      y += 1;
    if (A == D)
      x += 1;
    else if (B == D)
      y += 1;
    if (x <= 1)
      r += 1;
    if (y <= 1)
      r -= 1;
    return r;
}

static inline int GetResult2_32 (u32 A, u32 B, u32 C, u32 D,
                                 u32 /* E */)
{
  int x = 0;
  int y = 0;
  int r = 0;
  
  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r -= 1;
  if (y <= 1)
    r += 1;
  return r;
}

void Super2xSaI (u8 *srcPtr, u32 srcPitch,
                 u8 *deltaPtr, u8 *dstPtr, u32 dstPitch,
                 int width, int height)
{
  u16 *bP;
  u8  *dP;
  u32 inc_bP;
  u32 Nextline = srcPitch >> 1;
#ifdef MMX
  if (cpu_mmx) {
    for (; height; height--) {
      _2xSaISuper2xSaILine (srcPtr, deltaPtr, srcPitch, width,
                            dstPtr, dstPitch);
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }
  } else
#endif
    {
      inc_bP = 1;
      
      for (; height; height--) {
        bP = (u16 *) srcPtr;
        dP = (u8 *) dstPtr;
        
        for (u32 finish = width; finish; finish -= inc_bP) {
          u32 color4, color5, color6;
          u32 color1, color2, color3;
          u32 colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
          u32 product1a, product1b, product2a, product2b;
          
          //---------------------------------------    B1 B2
          //                                         4  5  6 S2
          //                                         1  2  3 S1
          //                                           A1 A2
          
          colorB0 = *(bP - Nextline - 1);
          colorB1 = *(bP - Nextline);
          colorB2 = *(bP - Nextline + 1);
          colorB3 = *(bP - Nextline + 2);
          
          color4 = *(bP - 1);
          color5 = *(bP);
          color6 = *(bP + 1);
          colorS2 = *(bP + 2);
          
          color1 = *(bP + Nextline - 1);
          color2 = *(bP + Nextline);
          color3 = *(bP + Nextline + 1);
          colorS1 = *(bP + Nextline + 2);
          
          colorA0 = *(bP + Nextline + Nextline - 1);
          colorA1 = *(bP + Nextline + Nextline);
          colorA2 = *(bP + Nextline + Nextline + 1);
          colorA3 = *(bP + Nextline + Nextline + 2);
          
          //--------------------------------------
          if (color2 == color6 && color5 != color3) {
            product2b = product1b = color2;
          } else if (color5 == color3 && color2 != color6) {
            product2b = product1b = color5;
          } else if (color5 == color3 && color2 == color6) {
            register int r = 0;
            
            r += GetResult (color6, color5, color1, colorA1);
            r += GetResult (color6, color5, color4, colorB1);
            r += GetResult (color6, color5, colorA2, colorS1);
            r += GetResult (color6, color5, colorB2, colorS2);
            
            if (r > 0)
              product2b = product1b = color6;
            else if (r < 0)
              product2b = product1b = color5;
            else {
              product2b = product1b = INTERPOLATE (color5, color6);
            }
          } else {
            if (color6 == color3 && color3 == colorA1
                && color2 != colorA2 && color3 != colorA0)
              product2b =
                Q_INTERPOLATE (color3, color3, color3, color2);
            else if (color5 == color2 && color2 == colorA2
                     && colorA1 != color3 && color2 != colorA3)
              product2b =
                Q_INTERPOLATE (color2, color2, color2, color3);
            else
              product2b = INTERPOLATE (color2, color3);
            
            if (color6 == color3 && color6 == colorB1
                && color5 != colorB2 && color6 != colorB0)
              product1b =
                Q_INTERPOLATE (color6, color6, color6, color5);
            else if (color5 == color2 && color5 == colorB2
                     && colorB1 != color6 && color5 != colorB3)
              product1b =
                Q_INTERPOLATE (color6, color5, color5, color5);
            else
              product1b = INTERPOLATE (color5, color6);
          }
          
          if (color5 == color3 && color2 != color6 && color4 == color5
              && color5 != colorA2)
            product2a = INTERPOLATE (color2, color5);
          else
            if (color5 == color1 && color6 == color5
                && color4 != color2 && color5 != colorA0)
              product2a = INTERPOLATE (color2, color5);
            else
              product2a = color2;
          
          if (color2 == color6 && color5 != color3 && color1 == color2
              && color2 != colorB2)
            product1a = INTERPOLATE (color2, color5);
          else
            if (color4 == color2 && color3 == color2
                && color1 != color5 && color2 != colorB0)
              product1a = INTERPOLATE (color2, color5);
            else
              product1a = color5;
          
          product1a = product1a | (product1b << 16);
          product2a = product2a | (product2b << 16);
          
          *((u32 *) dP) = product1a;
          *((u32 *) (dP + dstPitch)) = product2a;
          
          bP += inc_bP;
          dP += sizeof (u32);
        }                       // end of for ( finish= width etc..)
        
        srcPtr   += srcPitch;
        dstPtr   += dstPitch * 2;
        deltaPtr += srcPitch;
      }                 // endof: for (; height; height--)
    }
}

void Super2xSaI32 (u8 *srcPtr, u32 srcPitch,
                   u8 * /* deltaPtr */, u8 *dstPtr, u32 dstPitch,
                   int width, int height)
{
  u32 *bP;
  u32 *dP;
  u32 inc_bP;
  u32 Nextline = srcPitch >> 2;
  inc_bP = 1;
  
  for (; height; height--) {
    bP = (u32 *) srcPtr;
    dP = (u32 *) dstPtr;
    
    for (u32 finish = width; finish; finish -= inc_bP) {
      u32 color4, color5, color6;
      u32 color1, color2, color3;
      u32 colorA0, colorA1, colorA2, colorA3,
        colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
      u32 product1a, product1b, product2a, product2b;
      
      //---------------------------------------    B1 B2
      //                                         4  5  6 S2
      //                                         1  2  3 S1
      //                                           A1 A2
      
      colorB0 = *(bP - Nextline - 1);
      colorB1 = *(bP - Nextline);
      colorB2 = *(bP - Nextline + 1);
      colorB3 = *(bP - Nextline + 2);
      
      color4 = *(bP - 1);
      color5 = *(bP);
      color6 = *(bP + 1);
      colorS2 = *(bP + 2);
      
      color1 = *(bP + Nextline - 1);
      color2 = *(bP + Nextline);
      color3 = *(bP + Nextline + 1);
      colorS1 = *(bP + Nextline + 2);
      
      colorA0 = *(bP + Nextline + Nextline - 1);
      colorA1 = *(bP + Nextline + Nextline);
      colorA2 = *(bP + Nextline + Nextline + 1);
      colorA3 = *(bP + Nextline + Nextline + 2);
      
      //--------------------------------------
      if (color2 == color6 && color5 != color3) {
        product2b = product1b = color2;
      } else if (color5 == color3 && color2 != color6) {
        product2b = product1b = color5;
      } else if (color5 == color3 && color2 == color6) {
        register int r = 0;
        
        r += GetResult (color6, color5, color1, colorA1);
        r += GetResult (color6, color5, color4, colorB1);
        r += GetResult (color6, color5, colorA2, colorS1);
        r += GetResult (color6, color5, colorB2, colorS2);
        
        if (r > 0)
          product2b = product1b = color6;
        else if (r < 0)
          product2b = product1b = color5;
        else {
          product2b = product1b = INTERPOLATE (color5, color6);
        }
      } else {
        if (color6 == color3 && color3 == colorA1
            && color2 != colorA2 && color3 != colorA0)
          product2b =
            Q_INTERPOLATE (color3, color3, color3, color2);
        else if (color5 == color2 && color2 == colorA2
                 && colorA1 != color3 && color2 != colorA3)
          product2b =
            Q_INTERPOLATE (color2, color2, color2, color3);
        else
          product2b = INTERPOLATE (color2, color3);
        
        if (color6 == color3 && color6 == colorB1
            && color5 != colorB2 && color6 != colorB0)
          product1b =
            Q_INTERPOLATE (color6, color6, color6, color5);
        else if (color5 == color2 && color5 == colorB2
                 && colorB1 != color6 && color5 != colorB3)
          product1b =
            Q_INTERPOLATE (color6, color5, color5, color5);
        else
          product1b = INTERPOLATE (color5, color6);
      }
      
      if (color5 == color3 && color2 != color6 && color4 == color5
          && color5 != colorA2)
        product2a = INTERPOLATE (color2, color5);
      else
        if (color5 == color1 && color6 == color5
            && color4 != color2 && color5 != colorA0)
          product2a = INTERPOLATE (color2, color5);
        else
          product2a = color2;
      
      if (color2 == color6 && color5 != color3 && color1 == color2
          && color2 != colorB2)
        product1a = INTERPOLATE (color2, color5);
      else
        if (color4 == color2 && color3 == color2
            && color1 != color5 && color2 != colorB0)
          product1a = INTERPOLATE (color2, color5);
        else
          product1a = color5;
      *(dP) = product1a;
      *(dP+1) = product1b;
      *(dP + dstPitch/4) = product2a;
      *(dP + dstPitch/4 + 1) = product2b;
      
      bP += inc_bP;
      dP += 2;
    }                       // end of for ( finish= width etc..)
        
    srcPtr   += srcPitch;
    dstPtr   += dstPitch * 2;
    //        deltaPtr += srcPitch;
  }                 // endof: for (; height; height--)
}

void SuperEagle (u8 *srcPtr, u32 srcPitch, u8 *deltaPtr, 
                 u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8  *dP;
  u16 *bP;
  u16 *xP;
  u32 inc_bP;

#ifdef MMX
  if (cpu_mmx) {
    for (; height; height--) {
      _2xSaISuperEagleLine (srcPtr, deltaPtr, srcPitch, width,
                            dstPtr, dstPitch);
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }
  } else
#endif  
  {
    inc_bP = 1;
    
    u32 Nextline = srcPitch >> 1;
    
    for (; height; height--) {
      bP = (u16 *) srcPtr;
      xP = (u16 *) deltaPtr;
      dP = dstPtr;
      for (u32 finish = width; finish; finish -= inc_bP) {
        u32 color4, color5, color6;
        u32 color1, color2, color3;
        u32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
        u32 product1a, product1b, product2a, product2b;
        
        colorB1 = *(bP - Nextline);
        colorB2 = *(bP - Nextline + 1);
        
        color4 = *(bP - 1);
        color5 = *(bP);
        color6 = *(bP + 1);
        colorS2 = *(bP + 2);
        
        color1 = *(bP + Nextline - 1);
        color2 = *(bP + Nextline);
        color3 = *(bP + Nextline + 1);
        colorS1 = *(bP + Nextline + 2);
        
        colorA1 = *(bP + Nextline + Nextline);
        colorA2 = *(bP + Nextline + Nextline + 1);
        
        // --------------------------------------
        if (color2 == color6 && color5 != color3) {
          product1b = product2a = color2;
          if ((color1 == color2) || (color6 == colorB2)) {
            product1a = INTERPOLATE (color2, color5);
            product1a = INTERPOLATE (color2, product1a);
            //                       product1a = color2;
          } else {
            product1a = INTERPOLATE (color5, color6);
          }
          
          if ((color6 == colorS2) || (color2 == colorA1)) {
            product2b = INTERPOLATE (color2, color3);
            product2b = INTERPOLATE (color2, product2b);
            //                       product2b = color2;
          } else {
            product2b = INTERPOLATE (color2, color3);
          }
        } else if (color5 == color3 && color2 != color6) {
          product2b = product1a = color5;
          
          if ((colorB1 == color5) || (color3 == colorS1)) {
            product1b = INTERPOLATE (color5, color6);
            product1b = INTERPOLATE (color5, product1b);
            //                       product1b = color5;
          } else {
            product1b = INTERPOLATE (color5, color6);
          }
          
          if ((color3 == colorA2) || (color4 == color5)) {
            product2a = INTERPOLATE (color5, color2);
            product2a = INTERPOLATE (color5, product2a);
            //                       product2a = color5;
          } else {
            product2a = INTERPOLATE (color2, color3);
          }
          
        } else if (color5 == color3 && color2 == color6) {
          register int r = 0;
          
          r += GetResult (color6, color5, color1, colorA1);
          r += GetResult (color6, color5, color4, colorB1);
          r += GetResult (color6, color5, colorA2, colorS1);
          r += GetResult (color6, color5, colorB2, colorS2);
          
          if (r > 0) {
            product1b = product2a = color2;
            product1a = product2b = INTERPOLATE (color5, color6);
          } else if (r < 0) {
            product2b = product1a = color5;
            product1b = product2a = INTERPOLATE (color5, color6);
          } else {
            product2b = product1a = color5;
            product1b = product2a = color2;
          }
        } else {
          product2b = product1a = INTERPOLATE (color2, color6);
          product2b =
            Q_INTERPOLATE (color3, color3, color3, product2b);
          product1a =
            Q_INTERPOLATE (color5, color5, color5, product1a);
          
          product2a = product1b = INTERPOLATE (color5, color3);
          product2a =
            Q_INTERPOLATE (color2, color2, color2, product2a);
          product1b =
            Q_INTERPOLATE (color6, color6, color6, product1b);
          
          //                    product1a = color5;
          //                    product1b = color6;
          //                    product2a = color2;
          //                    product2b = color3;
        }
        product1a = product1a | (product1b << 16);
        product2a = product2a | (product2b << 16);
        
        *((u32 *) dP) = product1a;
        *((u32 *) (dP + dstPitch)) = product2a;
        *xP = color5;
        
        bP += inc_bP;
        xP += inc_bP;
        dP += sizeof (u32);
      }                 // end of for ( finish= width etc..)
      
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }                   // endof: for (height; height; height--)
  }
}

void SuperEagle32 (u8 *srcPtr, u32 srcPitch, u8 *deltaPtr, 
                   u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u32  *dP;
  u32 *bP;
  u32 *xP;
  u32 inc_bP;

  inc_bP = 1;
  
  u32 Nextline = srcPitch >> 2;
    
  for (; height; height--) {
    bP = (u32 *) srcPtr;
    xP = (u32 *) deltaPtr;
    dP = (u32 *)dstPtr;
    for (u32 finish = width; finish; finish -= inc_bP) {
      u32 color4, color5, color6;
      u32 color1, color2, color3;
      u32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
      u32 product1a, product1b, product2a, product2b;
      
      colorB1 = *(bP - Nextline);
      colorB2 = *(bP - Nextline + 1);
      
      color4 = *(bP - 1);
      color5 = *(bP);
      color6 = *(bP + 1);
      colorS2 = *(bP + 2);
      
      color1 = *(bP + Nextline - 1);
      color2 = *(bP + Nextline);
      color3 = *(bP + Nextline + 1);
      colorS1 = *(bP + Nextline + 2);
      
      colorA1 = *(bP + Nextline + Nextline);
      colorA2 = *(bP + Nextline + Nextline + 1);
      
      // --------------------------------------
      if (color2 == color6 && color5 != color3) {
        product1b = product2a = color2;
        if ((color1 == color2) || (color6 == colorB2)) {
          product1a = INTERPOLATE (color2, color5);
          product1a = INTERPOLATE (color2, product1a);
          //                       product1a = color2;
        } else {
          product1a = INTERPOLATE (color5, color6);
        }
        
        if ((color6 == colorS2) || (color2 == colorA1)) {
          product2b = INTERPOLATE (color2, color3);
          product2b = INTERPOLATE (color2, product2b);
          //                       product2b = color2;
        } else {
          product2b = INTERPOLATE (color2, color3);
        }
      } else if (color5 == color3 && color2 != color6) {
        product2b = product1a = color5;
        
        if ((colorB1 == color5) || (color3 == colorS1)) {
          product1b = INTERPOLATE (color5, color6);
          product1b = INTERPOLATE (color5, product1b);
          //                       product1b = color5;
        } else {
          product1b = INTERPOLATE (color5, color6);
        }
        
        if ((color3 == colorA2) || (color4 == color5)) {
          product2a = INTERPOLATE (color5, color2);
          product2a = INTERPOLATE (color5, product2a);
          //                       product2a = color5;
        } else {
          product2a = INTERPOLATE (color2, color3);
        }
        
      } else if (color5 == color3 && color2 == color6) {
        register int r = 0;
        
        r += GetResult (color6, color5, color1, colorA1);
        r += GetResult (color6, color5, color4, colorB1);
        r += GetResult (color6, color5, colorA2, colorS1);
        r += GetResult (color6, color5, colorB2, colorS2);
        
        if (r > 0) {
          product1b = product2a = color2;
          product1a = product2b = INTERPOLATE (color5, color6);
        } else if (r < 0) {
          product2b = product1a = color5;
          product1b = product2a = INTERPOLATE (color5, color6);
        } else {
          product2b = product1a = color5;
          product1b = product2a = color2;
        }
      } else {
        product2b = product1a = INTERPOLATE (color2, color6);
        product2b =
          Q_INTERPOLATE (color3, color3, color3, product2b);
        product1a =
          Q_INTERPOLATE (color5, color5, color5, product1a);
        
        product2a = product1b = INTERPOLATE (color5, color3);
        product2a =
          Q_INTERPOLATE (color2, color2, color2, product2a);
        product1b =
          Q_INTERPOLATE (color6, color6, color6, product1b);
        
        //                    product1a = color5;
        //                    product1b = color6;
        //                    product2a = color2;
        //                    product2b = color3;
      }
      *(dP) = product1a;
      *(dP+1) = product1b;
      *(dP + dstPitch/4) = product2a;
      *(dP + dstPitch/4+1) = product2b;
      *xP = color5;
      
      bP += inc_bP;
      xP += inc_bP;
      dP += 2;
    }                 // end of for ( finish= width etc..)
      
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    deltaPtr += srcPitch;
  }                   // endof: for (height; height; height--)
}

void _2xSaI (u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
             u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8  *dP;
  u16 *bP;
  u32 inc_bP;
  
#ifdef MMX
  if (cpu_mmx) {
    for (; height; height -= 1) {
      _2xSaILine (srcPtr, deltaPtr, srcPitch, width, dstPtr, dstPitch);
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }
  } else
#endif
  {
    inc_bP = 1;
    
    u32 Nextline = srcPitch >> 1;
    
    for (; height; height--) {
      bP = (u16 *) srcPtr;
      dP = dstPtr;
      
      for (u32 finish = width; finish; finish -= inc_bP) {
        
        register u32 colorA, colorB;
        u32 colorC, colorD,
          colorE, colorF, colorG, colorH,
          colorI, colorJ, colorK, colorL,
          
          colorM, colorN, colorO, colorP;
        u32 product, product1, product2;
        
        //---------------------------------------
        // Map of the pixels:                    I|E F|J
        //                                       G|A B|K
        //                                       H|C D|L
        //                                       M|N O|P
        colorI = *(bP - Nextline - 1);
        colorE = *(bP - Nextline);
        colorF = *(bP - Nextline + 1);
        colorJ = *(bP - Nextline + 2);
        
        colorG = *(bP - 1);
        colorA = *(bP);
        colorB = *(bP + 1);
        colorK = *(bP + 2);
        
        colorH = *(bP + Nextline - 1);
        colorC = *(bP + Nextline);
        colorD = *(bP + Nextline + 1);
        colorL = *(bP + Nextline + 2);
        
        colorM = *(bP + Nextline + Nextline - 1);
        colorN = *(bP + Nextline + Nextline);
        colorO = *(bP + Nextline + Nextline + 1);
        colorP = *(bP + Nextline + Nextline + 2);
        
        if ((colorA == colorD) && (colorB != colorC)) {
          if (((colorA == colorE) && (colorB == colorL)) ||
              ((colorA == colorC) && (colorA == colorF)
               && (colorB != colorE) && (colorB == colorJ))) {
            product = colorA;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }
          
          if (((colorA == colorG) && (colorC == colorO)) ||
              ((colorA == colorB) && (colorA == colorH)
               && (colorG != colorC) && (colorC == colorM))) {
            product1 = colorA;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
          product2 = colorA;
        } else if ((colorB == colorC) && (colorA != colorD)) {
          if (((colorB == colorF) && (colorA == colorH)) ||
              ((colorB == colorE) && (colorB == colorD)
               && (colorA != colorF) && (colorA == colorI))) {
            product = colorB;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }
          
          if (((colorC == colorH) && (colorA == colorF)) ||
              ((colorC == colorG) && (colorC == colorD)
               && (colorA != colorH) && (colorA == colorI))) {
            product1 = colorC;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
          product2 = colorB;
        } else if ((colorA == colorD) && (colorB == colorC)) {
          if (colorA == colorB) {
            product = colorA;
            product1 = colorA;
            product2 = colorA;
          } else {
            register int r = 0;
            
            product1 = INTERPOLATE (colorA, colorC);
            product = INTERPOLATE (colorA, colorB);
            
            r +=
              GetResult1 (colorA, colorB, colorG, colorE,
                          colorI);
            r +=
              GetResult2 (colorB, colorA, colorK, colorF,
                          colorJ);
            r +=
              GetResult2 (colorB, colorA, colorH, colorN,
                          colorM);
            r +=
              GetResult1 (colorA, colorB, colorL, colorO,
                          colorP);
            
            if (r > 0)
              product2 = colorA;
            else if (r < 0)
              product2 = colorB;
            else {
              product2 =
                Q_INTERPOLATE (colorA, colorB, colorC,
                               colorD);
            }
          }
        } else {
          product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);
          
          if ((colorA == colorC) && (colorA == colorF)
              && (colorB != colorE) && (colorB == colorJ)) {
            product = colorA;
          } else if ((colorB == colorE) && (colorB == colorD)
                     && (colorA != colorF) && (colorA == colorI)) {
            product = colorB;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }
          
          if ((colorA == colorB) && (colorA == colorH)
              && (colorG != colorC) && (colorC == colorM)) {
            product1 = colorA;
          } else if ((colorC == colorG) && (colorC == colorD)
                     && (colorA != colorH) && (colorA == colorI)) {
            product1 = colorC;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
        }
        
        product = colorA | (product << 16);
        product1 = product1 | (product2 << 16);
        *((s32 *) dP) = product;
        *((u32 *) (dP + dstPitch)) = product1;
        
        bP += inc_bP;
        dP += sizeof (u32);
      }                 // end of for ( finish= width etc..)
      
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }                   // endof: for (height; height; height--)
  }
}

void _2xSaI32 (u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
               u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u32  *dP;
  u32 *bP;
  u32 inc_bP = 1;
    
  u32 Nextline = srcPitch >> 2;
    
  for (; height; height--) {
    bP = (u32 *) srcPtr;
    dP = (u32 *) dstPtr;
      
    for (u32 finish = width; finish; finish -= inc_bP) {
      register u32 colorA, colorB;
      u32 colorC, colorD,
        colorE, colorF, colorG, colorH,
        colorI, colorJ, colorK, colorL,
        
        colorM, colorN, colorO, colorP;
      u32 product, product1, product2;
      
      //---------------------------------------
      // Map of the pixels:                    I|E F|J
      //                                       G|A B|K
      //                                       H|C D|L
      //                                       M|N O|P
      colorI = *(bP - Nextline - 1);
      colorE = *(bP - Nextline);
      colorF = *(bP - Nextline + 1);
      colorJ = *(bP - Nextline + 2);
        
      colorG = *(bP - 1);
      colorA = *(bP);
      colorB = *(bP + 1);
      colorK = *(bP + 2);
        
      colorH = *(bP + Nextline - 1);
      colorC = *(bP + Nextline);
      colorD = *(bP + Nextline + 1);
      colorL = *(bP + Nextline + 2);
      
      colorM = *(bP + Nextline + Nextline - 1);
      colorN = *(bP + Nextline + Nextline);
      colorO = *(bP + Nextline + Nextline + 1);
      colorP = *(bP + Nextline + Nextline + 2);
      
      if ((colorA == colorD) && (colorB != colorC)) {
        if (((colorA == colorE) && (colorB == colorL)) ||
            ((colorA == colorC) && (colorA == colorF)
             && (colorB != colorE) && (colorB == colorJ))) {
          product = colorA;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }
          
        if (((colorA == colorG) && (colorC == colorO)) ||
            ((colorA == colorB) && (colorA == colorH)
             && (colorG != colorC) && (colorC == colorM))) {
          product1 = colorA;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
        product2 = colorA;
      } else if ((colorB == colorC) && (colorA != colorD)) {
        if (((colorB == colorF) && (colorA == colorH)) ||
            ((colorB == colorE) && (colorB == colorD)
             && (colorA != colorF) && (colorA == colorI))) {
          product = colorB;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }
        
        if (((colorC == colorH) && (colorA == colorF)) ||
            ((colorC == colorG) && (colorC == colorD)
             && (colorA != colorH) && (colorA == colorI))) {
          product1 = colorC;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
        product2 = colorB;
      } else if ((colorA == colorD) && (colorB == colorC)) {
        if (colorA == colorB) {
          product = colorA;
          product1 = colorA;
          product2 = colorA;
        } else {
          register int r = 0;
          
          product1 = INTERPOLATE (colorA, colorC);
          product = INTERPOLATE (colorA, colorB);
          
          r +=
            GetResult1 (colorA, colorB, colorG, colorE,
                        colorI);
          r +=
            GetResult2 (colorB, colorA, colorK, colorF,
                        colorJ);
          r +=
            GetResult2 (colorB, colorA, colorH, colorN,
                           colorM);
          r +=
            GetResult1 (colorA, colorB, colorL, colorO,
                           colorP);
            
          if (r > 0)
            product2 = colorA;
          else if (r < 0)
            product2 = colorB;
          else {
            product2 =
              Q_INTERPOLATE (colorA, colorB, colorC,
                               colorD);
            }
        }
      } else {
        product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);
        
        if ((colorA == colorC) && (colorA == colorF)
            && (colorB != colorE) && (colorB == colorJ)) {
          product = colorA;
        } else if ((colorB == colorE) && (colorB == colorD)
                   && (colorA != colorF) && (colorA == colorI)) {
          product = colorB;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }
        
        if ((colorA == colorB) && (colorA == colorH)
            && (colorG != colorC) && (colorC == colorM)) {
          product1 = colorA;
        } else if ((colorC == colorG) && (colorC == colorD)
                   && (colorA != colorH) && (colorA == colorI)) {
          product1 = colorC;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
      }
      *(dP) = colorA;
      *(dP + 1) = product;
      *(dP + dstPitch/4) = product1;
      *(dP + dstPitch/4 + 1) = product2;
      
      bP += inc_bP;
      dP += 2;
    }                 // end of for ( finish= width etc..)
    
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    //    deltaPtr += srcPitch;
  }                   // endof: for (height; height; height--)
}

static u32 Bilinear (u32 A, u32 B, u32 x)
{
  unsigned long areaA, areaB;
  unsigned long result;
  
  if (A == B)
    return A;
  
  areaB = (x >> 11) & 0x1f;     // reduce 16 bit fraction to 5 bits
  areaA = 0x20 - areaB;
  
  A = (A & redblueMask) | ((A & greenMask) << 16);
  B = (B & redblueMask) | ((B & greenMask) << 16);
  
  result = ((areaA * A) + (areaB * B)) >> 5;
  
  return (result & redblueMask) | ((result >> 16) & greenMask);
}

static u32 Bilinear4 (u32 A, u32 B, u32 C, u32 D, u32 x,
                         u32 y)
{
  unsigned long areaA, areaB, areaC, areaD;
  unsigned long result, xy;
  
  x = (x >> 11) & 0x1f;
  y = (y >> 11) & 0x1f;
  xy = (x * y) >> 5;
  
  A = (A & redblueMask) | ((A & greenMask) << 16);
  B = (B & redblueMask) | ((B & greenMask) << 16);
  C = (C & redblueMask) | ((C & greenMask) << 16);
  D = (D & redblueMask) | ((D & greenMask) << 16);
  
  areaA = 0x20 + xy - x - y;
  areaB = x - xy;
  areaC = y - xy;
  areaD = xy;
  
  result = ((areaA * A) + (areaB * B) + (areaC * C) + (areaD * D)) >> 5;
  
  return (result & redblueMask) | ((result >> 16) & greenMask);
}

void Scale_2xSaI (u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                  u8 *dstPtr, u32 dstPitch, 
                  u32 dstWidth, u32 dstHeight, int width, int height)
{
  u8  *dP;
  u16 *bP;

  u32 w;
  u32 h;
  u32 dw;
  u32 dh;
  u32 hfinish;
  u32 wfinish;
  
  u32 Nextline = srcPitch >> 1;
  
  wfinish = (width - 1) << 16;  // convert to fixed point
  dw = wfinish / (dstWidth - 1);
  hfinish = (height - 1) << 16; // convert to fixed point
  dh = hfinish / (dstHeight - 1);
  
  for (h = 0; h < hfinish; h += dh) {
    u32 y1, y2;
    
    y1 = h & 0xffff;    // fraction part of fixed point
    bP = (u16 *) (srcPtr + ((h >> 16) * srcPitch));
    dP = dstPtr;
    y2 = 0x10000 - y1;
    
    w = 0;
    
    for (; w < wfinish;) {
      u32 A, B, C, D;
      u32 E, F, G, H;
      u32 I, J, K, L;
      u32 x1, x2, a1, f1, f2;
      u32 position, product1;
      
      position = w >> 16;
      A = bP[position]; // current pixel
      B = bP[position + 1];     // next pixel
      C = bP[position + Nextline];
      D = bP[position + Nextline + 1];
      E = bP[position - Nextline];
      F = bP[position - Nextline + 1];
      G = bP[position - 1];
      H = bP[position + Nextline - 1];
      I = bP[position + 2];
      J = bP[position + Nextline + 2];
      K = bP[position + Nextline + Nextline];
      L = bP[position + Nextline + Nextline + 1];
      
      x1 = w & 0xffff;  // fraction part of fixed point
      x2 = 0x10000 - x1;
      
      /*0*/ 
      if (A == B && C == D && A == C)
        product1 = A;
      else /*1*/ if (A == D && B != C) {
        f1 = (x1 >> 1) + (0x10000 >> 2);
        f2 = (y1 >> 1) + (0x10000 >> 2);
        if (y1 <= f1 && A == J && A != E)       // close to B
          {
            a1 = f1 - y1;
            product1 = Bilinear (A, B, a1);
          } else if (y1 >= f1 && A == G && A != L)      // close to C
            {
              a1 = y1 - f1;
              product1 = Bilinear (A, C, a1);
            }
        else if (x1 >= f2 && A == E && A != J)  // close to B
          {
            a1 = x1 - f2;
            product1 = Bilinear (A, B, a1);
          }
        else if (x1 <= f2 && A == L && A != G)  // close to C
          {
            a1 = f2 - x1;
            product1 = Bilinear (A, C, a1);
          }
        else if (y1 >= x1)      // close to C
          {
            a1 = y1 - x1;
            product1 = Bilinear (A, C, a1);
          }
        else if (y1 <= x1)      // close to B
          {
            a1 = x1 - y1;
            product1 = Bilinear (A, B, a1);
          }
      }
      else
        /*2*/ 
        if (B == C && A != D)
          {
            f1 = (x1 >> 1) + (0x10000 >> 2);
            f2 = (y1 >> 1) + (0x10000 >> 2);
            if (y2 >= f1 && B == H && B != F)   // close to A
              {
                a1 = y2 - f1;
                product1 = Bilinear (B, A, a1);
              }
            else if (y2 <= f1 && B == I && B != K)      // close to D
              {
                a1 = f1 - y2;
                product1 = Bilinear (B, D, a1);
              }
            else if (x2 >= f2 && B == F && B != H)      // close to A
              {
                a1 = x2 - f2;
                product1 = Bilinear (B, A, a1);
              }
            else if (x2 <= f2 && B == K && B != I)      // close to D
              {
                a1 = f2 - x2;
                product1 = Bilinear (B, D, a1);
              }
            else if (y2 >= x1)  // close to A
              {
                a1 = y2 - x1;
                product1 = Bilinear (B, A, a1);
              }
            else if (y2 <= x1)  // close to D
              {
                a1 = x1 - y2;
                product1 = Bilinear (B, D, a1);
              }
          }
      /*3*/
        else
          {
            product1 = Bilinear4 (A, B, C, D, x1, y1);
          }
      
      //end First Pixel
      *(u32 *) dP = product1;
      dP += 2;
      w += dw;
    }
    dstPtr += dstPitch;
  }
}

/* TvMode filter (VisualBoy Advance, Forgotten) */
void TVMode (u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
             u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~(RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 16));
  u32 lowPixelMask = RGB_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    u32 currentDelta;
    u32 nextDelta;
    
    finish = (u8 *) bP + ((width+1) << 1);
    nextPixel = *bP++;
    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      currentDelta = nextDelta;
      nextPixel = *bP++;
      nextDelta = *xP++;
      
      if ((nextPixel != nextDelta) || (currentPixel != currentDelta)) {
        u32 colorA, colorB, product, darkened;
        
        *(xP - 2) = currentPixel;
#ifdef WORDS_BIGENDIAN
        colorA = currentPixel >> 16;
        colorB = (currentPixel << 16) >> 16;
#else
        colorA = currentPixel & 0xffff;
        colorB = (currentPixel & 0xffff0000) >> 16;
#endif

        *(dP) = product = colorA |
          ((((colorA & colorMask) >> 1) +
            ((colorB & colorMask) >> 1) +
            (colorA & colorB & lowPixelMask)) << 16);
        darkened = (product = ((product & colorMask) >> 1));
        darkened += (product = ((product & colorMask) >> 1));
        darkened += (product & colorMask) >> 1;
        *(nL) = darkened;

#ifdef WORDS_BIGENDIAN
        colorA = nextPixel >> 16;
#else
        colorA = nextPixel & 0xffff;
#endif

        *(dP + 1) = product = colorB |
          ((((colorA & colorMask) >> 1) +
            ((colorB & colorMask) >> 1) +
            (colorA & colorB & lowPixelMask)) << 16);
        darkened = (product = ((product & colorMask) >> 1));
        darkened += (product = ((product & colorMask) >> 1));
        darkened += (product & colorMask) >> 1;
        *(nL + 1) = darkened;
      }
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

void TVMode32(u8 *srcPtr, u32 srcPitch, u8 */* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~(RGB32_LOW_BITS_MASK);
  u32 lowPixelMask = RGB32_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    //    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    //    u32 currentDelta;
    //    u32 nextDelta;
    
    finish = (u8 *) bP + ((width+1) << 2);
    nextPixel = *bP++;
    //    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      //      currentDelta = nextDelta;
      nextPixel = *bP++;
      //      nextDelta = *xP++;
      
      u32 colorA, colorB, product, darkened;
        
      //      *(xP - 2) = currentPixel;
      colorA = currentPixel;
      colorB = nextPixel;

      *(dP) = colorA;
      *(dP+1) = product = (((colorA & colorMask) >> 1) +
                           ((colorB & colorMask) >> 1) +
                           (colorA & colorB & lowPixelMask));
      darkened = (product = ((product & colorMask) >> 1));
      darkened += (product = ((product & colorMask) >> 1));
      //      darkened += (product & colorMask) >> 1;
      product = (colorA & colorMask) >> 1;
      product += (product & colorMask) >> 1;
      //      product += (product & colorMask) >> 1;
      *(nL) = product;
      *(nL+1) = darkened;

      nextPixel = *bP++;
      colorA = nextPixel;

      *(dP + 2) = colorB;
      *(dP + 3) = product = 
        (((colorA & colorMask) >> 1) +
         ((colorB & colorMask) >> 1) +
         (colorA & colorB & lowPixelMask));
      darkened = (product = ((product & colorMask) >> 1));
      darkened += (product = ((product & colorMask) >> 1));
      //      darkened += (product & colorMask) >> 1;
      product = (colorB & colorMask) >> 1;
      product += (product & colorMask) >> 1;
      //      product += (product & colorMask) >> 1;
      *(nL+2) = product;      
      *(nL+3) = darkened;
      
      dP += 4;
      nL += 4;
    } while ((u8 *) bP < finish);
    
    //    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch*2;
    nextLine += dstPitch*2;
  }
  while (--height);
}

/* Pixelate filter (VisualBoy Advance, Forgotten) */

void Pixelate(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
          u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~(RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 16));
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    u32 currentDelta;
    u32 nextDelta;
    
    finish = (u8 *) bP + ((width+1) << 1);
    nextPixel = *bP++;
    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      currentDelta = nextDelta;
      nextPixel = *bP++;
      nextDelta = *xP++;
      
      if ((nextPixel != nextDelta) || (currentPixel != currentDelta)) {
        u32 colorA, colorB, product;
        
        *(xP - 2) = currentPixel;
#ifdef WORDS_BIGENDIAN
        colorA = currentPixel >> 16;
        colorB = (currentPixel << 16) >> 16;
#else
        colorA = currentPixel & 0xffff;
        colorB = (currentPixel & 0xffff0000) >> 16;
#endif

        product = (((colorA & colorMask) >> 1) & colorMask) >> 1;
        *(nL) = product | (product << 16);
        *(dP) = (colorA)|product<<16;
        
#ifdef WORDS_BIGENDIAN
        colorA = nextPixel >> 16;
#else
        colorA = nextPixel & 0xffff;
#endif
        product = (((colorB & colorMask) >> 1) & colorMask) >> 1;
        *(nL + 1) = product | (product << 16);
        *(dP + 1) = (colorB)| product<<16;
      }
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

void Pixelate32(u8 *srcPtr, u32 srcPitch, u8 */* deltaPtr */,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~RGB32_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    //    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    
    finish = (u8 *) bP + ((width+1) << 2);
    nextPixel = *bP++;
    
    do {
      currentPixel = nextPixel;
      nextPixel = *bP++;
      
      u32 colorA, colorB, product;
        
      colorA = currentPixel;
      colorB = nextPixel;

      product = (((colorA & colorMask) >> 1) & colorMask) >> 1;
      *(nL) = product;
      *(nL+1) = product;
      *(dP) = colorA;
      *(dP+1) = product;

      nextPixel = *bP++;
      colorA = nextPixel;
      product = (((colorB & colorMask) >> 1) & colorMask) >> 1;
      *(nL + 2) = product;
      *(nL + 3) = product;
      *(dP + 2) = colorB;
      *(dP + 3) = product;
      
      dP += 4;
      nL += 4;
    } while ((u8 *) bP < finish);
    
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

/* MotionBlur filter (VisualBoy Advance, Forgotten) */

void MotionBlur(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~(RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 16));
  u32 lowPixelMask = RGB_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    u32 currentDelta;
    u32 nextDelta;
    
    finish = (u8 *) bP + ((width+1) << 1);
    nextPixel = *bP++;
    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      currentDelta = nextDelta;
      nextPixel = *bP++;
      nextDelta = *xP++;

      if(currentPixel != currentDelta) {
        u32 colorA, product, colorB;
        
        *(xP - 2) = currentPixel;
#ifdef WORDS_BIGENDIAN
        colorA = currentPixel >> 16;
        colorB = (currentDelta >> 16);
#else
        colorA = currentPixel & 0xffff;
        colorB = (currentDelta & 0xffff);        
#endif

        product =   ((((colorA & colorMask) >> 1) +
                          ((colorB & colorMask) >> 1) +
                          (colorA & colorB & lowPixelMask)));
        
        *(dP) = product | product << 16;
        *(nL) = product | product << 16;

#ifdef WORDS_BIGENDIAN
        colorA = (currentPixel << 16) >> 16;
        colorB = (currentDelta << 16) >> 16;
#else
        colorA = currentPixel >> 16;
        colorB = currentDelta >> 16;
#endif
        product = ((((colorA & colorMask) >> 1) +
                  ((colorB & colorMask) >> 1) +
                    (colorA & colorB & lowPixelMask)));
        
        *(dP + 1) = product | product << 16;
        *(nL + 1) = product | product << 16;
      } else {
        u32 colorA, product;
        
        *(xP - 2) = currentPixel;
#ifdef WORDS_BIGENDIAN
        colorA = (currentPixel >> 16);
#else
        colorA = currentPixel & 0xffff;
#endif
        
        product = colorA;
        
        *(dP) = product | product << 16;
        *(nL) = product | product << 16;
#ifdef WORDS_BIGENDIAN
        colorA = (currentPixel << 16) >> 16;
#else
        colorA = currentPixel >> 16;
#endif
        product = colorA;
        
        *(dP + 1) = product | product << 16;
        *(nL + 1) = product | product << 16;        
      }
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

void MotionBlur32(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~RGB32_LOW_BITS_MASK;
  u32 lowPixelMask = RGB32_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    u32 currentDelta;
    u32 nextDelta;
    
    finish = (u8 *) bP + ((width+1) << 2);
    nextPixel = *bP++;
    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      currentDelta = nextDelta;
      nextPixel = *bP++;
      nextDelta = *xP++;

      u32 colorA, product, colorB;

      *(xP - 2) = currentPixel;
      colorA = currentPixel;
      colorB = currentDelta;
      
      product =   ((((colorA & colorMask) >> 1) +
                    ((colorB & colorMask) >> 1) +
                    (colorA & colorB & lowPixelMask)));
      
      *(dP) = product;
      *(dP+1) = product;
      *(nL) = product;
      *(nL+1) = product;

      *(xP - 1) = nextPixel;

      colorA = nextPixel;
      colorB = nextDelta;
      
      product = ((((colorA & colorMask) >> 1) +
                  ((colorB & colorMask) >> 1) +
                  (colorA & colorB & lowPixelMask)));
      
      *(dP + 2) = product;
      *(dP + 3) = product;
      *(nL + 2) = product;
      *(nL + 3) = product;

      nextPixel = *bP++;
      nextDelta = *xP++;
      
      dP += 4;
      nL += 4;
    } while ((u8 *) bP < finish);
    
    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

/* Simple2x filter (VisualBoy Advance, Forgotten) */

void Simple2x(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    
    finish = (u8 *) bP + ((width+1) << 1);
    currentPixel = *bP++;
    
    do {
#ifdef WORDS_BIGENDIAN
      u32 color = currentPixel >> 16;
#else
      u32 color = currentPixel & 0xffff;
#endif

      color = color | (color << 16);

      *(dP) = color;
      *(nL) = color;

#ifdef WORDS_BIGENDIAN
      color = (currentPixel << 16) >> 16;
#else
      color = currentPixel >> 16;
#endif
      color = color| (color << 16);      
      *(dP + 1) = color;
      *(nL + 1) = color;
      
      currentPixel = *bP++;
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2 * 2; // fix for retrogame
    nextLine += dstPitch * 2 * 2; // fix for retrogame
  }
  while (--height);
}

void Simple2x32(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    
    finish = (u8 *) bP + ((width+1) << 2);
    currentPixel = *bP++;
    
    do {
      u32 color = currentPixel;

      *(dP) = color;
      *(dP+1) = color;
      *(nL) = color;
      *(nL + 1) = color;
      
      currentPixel = *bP++;
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

/* Bilinear filter (VisualBoy Advance, Forgotten) */

static void fill_rgb_row_16(u16 *from, int src_width, u8 *row, int width )
{
  u8 *copy_start = row + src_width*3;
  u8 *all_stop = row + width*3;
  while (row < copy_start) {
    u16 color = *from++;
    *row++ = ((color >> systemRedShift) & 0x1f) << 3;
    *row++ = ((color >> systemGreenShift) & 0x1f) << 3;
    *row++ = ((color >> systemBlueShift) & 0x1f) << 3;
  }
  // any remaining elements to be written to 'row' are a replica of the
  // preceding pixel
  u8 *p = row-3;
  while (row < all_stop) {
    // we're guaranteed three elements per pixel; could unroll the loop
    // further, especially with a Duff's Device, but the gains would be
    // probably limited (judging by profiler output)
    *row++ = *p++;
    *row++ = *p++;
    *row++ = *p++;
  }
}

static void fill_rgb_row_32(u32 *from, int src_width, u8 *row, int width)
{
  u8 *copy_start = row + src_width*3;
  u8 *all_stop = row + width*3;
  while (row < copy_start) {
    u32 color = *from++;
    *row++ = ((color >> systemRedShift) & 0x1f) << 3;
    *row++ = ((color >> systemGreenShift) & 0x1f) << 3;
    *row++ = ((color >> systemBlueShift) & 0x1f) << 3;
  }
  // any remaining elements to be written to 'row' are a replica of the
  // preceding pixel
  u8 *p = row-3;
  while (row < all_stop) {
    // we're guaranteed three elements per pixel; could unroll the loop
    // further, especially with a Duff's Device, but the gains would be
    // probably limited (judging by profiler output)
    *row++ = *p++;
    *row++ = *p++;
    *row++ = *p++;
  }
}

void bilinear(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u16 *to = (u16 *)dstPtr;
  u16 *to_odd = (u16 *)(dstPtr + dstPitch);

  int from_width = width;
  if(width+1 < from_width)
    from_width = width+1;
  u16 *from = (u16 *)srcPtr;
  fill_rgb_row_16(from, from_width, rgb_row_cur, width+1);

  for(int y = 0; y < height; y++) {
    u16 *from_orig = from;
    u16 *to_orig = to;
    
    if (y+1 < height)
      fill_rgb_row_16(from+width+1, from_width, rgb_row_next, 
                   width+1);
    else
      fill_rgb_row_16(from, from_width, rgb_row_next, width+1);
    
    // every pixel in the src region, is extended to 4 pixels in the
    // destination, arranged in a square 'quad'; if the current src
    // pixel is 'a', then in what follows 'b' is the src pixel to the
    // right, 'c' is the src pixel below, and 'd' is the src pixel to
    // the right and down
    u8 *cur_row  = rgb_row_cur;
    u8 *next_row = rgb_row_next;
    u8 *ar = cur_row++;
    u8 *ag = cur_row++;
    u8 *ab = cur_row++;
    u8 *cr = next_row++;
    u8 *cg = next_row++;
    u8 *cb = next_row++;
    for(int x=0; x < width; x++) {
      u8 *br = cur_row++;
      u8 *bg = cur_row++;
      u8 *bb = cur_row++;
      u8 *dr = next_row++;
      u8 *dg = next_row++;
      u8 *db = next_row++;

      // upper left pixel in quad: just copy it in
      *to++ = RGB(*ar, *ag, *ab);
      
      // upper right
      *to++ = RGB((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);
      
      // lower left
      *to_odd++ = RGB((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);
      
      // lower right
      *to_odd++ = RGB((*ar+*br+*cr+*dr)>>2,
                      (*ag+*bg+*cg+*dg)>>2,
                      (*ab+*bb+*cb+*db)>>2);
      
      // 'b' becomes 'a', 'd' becomes 'c'
      ar = br;
      ag = bg;
      ab = bb;
      cr = dr;
      cg = dg;
      cb = db;
    }
    
    // the "next" rgb row becomes the current; the old current rgb row is
    // recycled and serves as the new "next" row
    u8 *temp;
    temp = rgb_row_cur;
    rgb_row_cur = rgb_row_next;
    rgb_row_next = temp;
    
    // update the pointers for start of next pair of lines
    from = (u16 *)((u8 *)from_orig + srcPitch);
    to = (u16 *)((u8 *)to_orig + 2*dstPitch);
    to_odd = (u16 *)((u8 *)to + dstPitch);
  }
}

void bilinearPlus(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                  u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u16 *to = (u16 *)dstPtr;
  u16 *to_odd = (u16 *)(dstPtr + dstPitch);

  int from_width = width;
  if(width+1 < from_width)
    from_width = width+1;
  u16 *from = (u16 *)srcPtr;
  fill_rgb_row_16(from, from_width, rgb_row_cur, width+1);

  for(int y = 0; y < height; y++) {
    u16 *from_orig = from;
    u16 *to_orig = to;
    
    if (y+1 < height)
      fill_rgb_row_16(from+width+1, from_width, rgb_row_next, 
                   width+1);
    else
      fill_rgb_row_16(from, from_width, rgb_row_next, width+1);
    
    // every pixel in the src region, is extended to 4 pixels in the
    // destination, arranged in a square 'quad'; if the current src
    // pixel is 'a', then in what follows 'b' is the src pixel to the
    // right, 'c' is the src pixel below, and 'd' is the src pixel to
    // the right and down
    u8 *cur_row  = rgb_row_cur;
    u8 *next_row = rgb_row_next;
    u8 *ar = cur_row++;
    u8 *ag = cur_row++;
    u8 *ab = cur_row++;
    u8 *cr = next_row++;
    u8 *cg = next_row++;
    u8 *cb = next_row++;
    for(int x=0; x < width; x++) {
      u8 *br = cur_row++;
      u8 *bg = cur_row++;
      u8 *bb = cur_row++;
      u8 *dr = next_row++;
      u8 *dg = next_row++;
      u8 *db = next_row++;
      
      // upper left pixel in quad: just copy it in
      //*to++ = manip.rgb(*ar, *ag, *ab);
#ifdef USE_ORIGINAL_BILINEAR_PLUS
      *to++ = RGB(
                  (((*ar)<<2) +((*ar)) + (*cr+*br+*br) )>> 3,
                  (((*ag)<<2) +((*ag)) + (*cg+*bg+*bg) )>> 3,
                  (((*ab)<<2) +((*ab)) + (*cb+*bb+*bb) )>> 3);
#else
      *to++ = RGB(
                  (((*ar)<<3) +((*ar)<<1) + (*cr+*br+*br+*cr) )>> 4,
                  (((*ag)<<3) +((*ag)<<1) + (*cg+*bg+*bg+*cg) )>> 4,
                  (((*ab)<<3) +((*ab)<<1) + (*cb+*bb+*bb+*cb) )>> 4);
#endif
      
      // upper right
      *to++ = RGB((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);
      
      // lower left
      *to_odd++ = RGB((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);
      
      // lower right
      *to_odd++ = RGB((*ar+*br+*cr+*dr)>>2,
                      (*ag+*bg+*cg+*dg)>>2,
                      (*ab+*bb+*cb+*db)>>2);
      
      // 'b' becomes 'a', 'd' becomes 'c'
      ar = br;
      ag = bg;
      ab = bb;
      cr = dr;
      cg = dg;
      cb = db;
    }
    
    // the "next" rgb row becomes the current; the old current rgb row is
    // recycled and serves as the new "next" row
    u8 *temp;
    temp = rgb_row_cur;
    rgb_row_cur = rgb_row_next;
    rgb_row_next = temp;
    
    // update the pointers for start of next pair of lines
    from = (u16 *)((u8 *)from_orig + srcPitch);
    to = (u16 *)((u8 *)to_orig + 2*dstPitch);
    to_odd = (u16 *)((u8 *)to + dstPitch);
  }
}

void bilinear32(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u32 *to = (u32 *)dstPtr;
  u32 *to_odd = (u32 *)(dstPtr + dstPitch);

  int from_width = width;
  if(width+1 < from_width)
    from_width = width+1;
  u32 *from = (u32 *)srcPtr;
  fill_rgb_row_32(from, from_width, rgb_row_cur, width+1);

  for(int y = 0; y < height; y++) {
    u32 *from_orig = from;
    u32 *to_orig = to;
    
    if (y+1 < height)
      fill_rgb_row_32(from+width+1, from_width, rgb_row_next, 
                   width+1);
    else
      fill_rgb_row_32(from, from_width, rgb_row_next, width+1);
    
    // every pixel in the src region, is extended to 4 pixels in the
    // destination, arranged in a square 'quad'; if the current src
    // pixel is 'a', then in what follows 'b' is the src pixel to the
    // right, 'c' is the src pixel below, and 'd' is the src pixel to
    // the right and down
    u8 *cur_row  = rgb_row_cur;
    u8 *next_row = rgb_row_next;
    u8 *ar = cur_row++;
    u8 *ag = cur_row++;
    u8 *ab = cur_row++;
    u8 *cr = next_row++;
    u8 *cg = next_row++;
    u8 *cb = next_row++;
    for(int x=0; x < width; x++) {
      u8 *br = cur_row++;
      u8 *bg = cur_row++;
      u8 *bb = cur_row++;
      u8 *dr = next_row++;
      u8 *dg = next_row++;
      u8 *db = next_row++;

      // upper left pixel in quad: just copy it in
      *to++ = RGB(*ar, *ag, *ab);
      
      // upper right
      *to++ = RGB((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);
      
      // lower left
      *to_odd++ = RGB((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);
      
      // lower right
      *to_odd++ = RGB((*ar+*br+*cr+*dr)>>2,
                      (*ag+*bg+*cg+*dg)>>2,
                      (*ab+*bb+*cb+*db)>>2);
      
      // 'b' becomes 'a', 'd' becomes 'c'
      ar = br;
      ag = bg;
      ab = bb;
      cr = dr;
      cg = dg;
      cb = db;
    }
    
    // the "next" rgb row becomes the current; the old current rgb row is
    // recycled and serves as the new "next" row
    u8 *temp;
    temp = rgb_row_cur;
    rgb_row_cur = rgb_row_next;
    rgb_row_next = temp;
    
    // update the pointers for start of next pair of lines
    from = (u32 *)((u8 *)from_orig + srcPitch);
    to = (u32 *)((u8 *)to_orig + 2*dstPitch);
    to_odd = (u32 *)((u8 *)to + dstPitch);
  }
}

void bilinearPlus32(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                    u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u32 *to = (u32 *)dstPtr;
  u32 *to_odd = (u32 *)(dstPtr + dstPitch);

  int from_width = width;
  if(width+1 < from_width)
    from_width = width+1;
  u32 *from = (u32 *)srcPtr;
  fill_rgb_row_32(from, from_width, rgb_row_cur, width+1);

  for(int y = 0; y < height; y++) {
    u32 *from_orig = from;
    u32 *to_orig = to;
    
    if (y+1 < height)
      fill_rgb_row_32(from+width+1, from_width, rgb_row_next, 
                   width+1);
    else
      fill_rgb_row_32(from, from_width, rgb_row_next, width+1);
    
    // every pixel in the src region, is extended to 4 pixels in the
    // destination, arranged in a square 'quad'; if the current src
    // pixel is 'a', then in what follows 'b' is the src pixel to the
    // right, 'c' is the src pixel below, and 'd' is the src pixel to
    // the right and down
    u8 *cur_row  = rgb_row_cur;
    u8 *next_row = rgb_row_next;
    u8 *ar = cur_row++;
    u8 *ag = cur_row++;
    u8 *ab = cur_row++;
    u8 *cr = next_row++;
    u8 *cg = next_row++;
    u8 *cb = next_row++;
    for(int x=0; x < width; x++) {
      u8 *br = cur_row++;
      u8 *bg = cur_row++;
      u8 *bb = cur_row++;
      u8 *dr = next_row++;
      u8 *dg = next_row++;
      u8 *db = next_row++;
      
      // upper left pixel in quad: just copy it in
      //*to++ = manip.rgb(*ar, *ag, *ab);
#ifdef USE_ORIGINAL_BILINEAR_PLUS
      *to++ = RGB(
                  (((*ar)<<2) +((*ar)) + (*cr+*br+*br) )>> 3,
                  (((*ag)<<2) +((*ag)) + (*cg+*bg+*bg) )>> 3,
                  (((*ab)<<2) +((*ab)) + (*cb+*bb+*bb) )>> 3);
#else
      *to++ = RGB(
                  (((*ar)<<3) +((*ar)<<1) + (*cr+*br+*br+*cr) )>> 4,
                  (((*ag)<<3) +((*ag)<<1) + (*cg+*bg+*bg+*cg) )>> 4,
                  (((*ab)<<3) +((*ab)<<1) + (*cb+*bb+*bb+*cb) )>> 4);
#endif
      
      // upper right
      *to++ = RGB((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);
      
      // lower left
      *to_odd++ = RGB((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);
      
      // lower right
      *to_odd++ = RGB((*ar+*br+*cr+*dr)>>2,
                      (*ag+*bg+*cg+*dg)>>2,
                      (*ab+*bb+*cb+*db)>>2);
      
      // 'b' becomes 'a', 'd' becomes 'c'
      ar = br;
      ag = bg;
      ab = bb;
      cr = dr;
      cg = dg;
      cb = db;
    }
    
    // the "next" rgb row becomes the current; the old current rgb row is
    // recycled and serves as the new "next" row
    u8 *temp;
    temp = rgb_row_cur;
    rgb_row_cur = rgb_row_next;
    rgb_row_next = temp;
    
    // update the pointers for start of next pair of lines
    from = (u32 *)((u8 *)from_orig + srcPitch);
    to = (u32 *)((u8 *)to_orig + 2*dstPitch);
    to_odd = (u32 *)((u8 *)to + dstPitch);
  }
}

/* Bilinear filter (VisualBoy Advance, Forgotten) */

inline unsigned short average(unsigned short a, unsigned short b)
{
	return (unsigned short)(((a ^ b) & 0xf7deU) >> 1) + (a & b);
	/* http://www.compuphase.com/menu.htm */
}

void Average(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */, u8 *dstPtr, u32 dstPitch, int width, int height)
{
	u16 *ssrc = (u16 *)srcPtr;
	u16 *ddst = (u16 *)dstPtr;
	int srowbytes = srcPitch >> 1, drowbytes = dstPitch >> 1;
	int x, y;
	
	for(y = 0; y < height; y++) {
		uint16 *s = (uint16 *)ssrc;
		uint16 *s1 = (uint16 *)ssrc + srowbytes;
		uint16 *d = (uint16 *)ddst;
		uint16 *d1 = (uint16 *)ddst + drowbytes;
		
		for(x = 0; x < width; x++) {
			uint16 pixelA = *(s);
        	uint16 pixelC = *(s + 1);
        	d[0] = pixelA;
        	d[1] = average(pixelA, pixelC);
        	s += 1;
        	d += 2;
        }
        s -= srowbytes;
        
        for(x = 0; x < width; x++) {
			uint16 pixelA = *(s);
			uint16 pixelC = *(s + 1);
			uint16 pixelI = *(s1);
			uint16 pixelJ = *(s1 + 1);
			
			uint16 pixelB = average(pixelA, pixelC);
			
			d1[0] = average(pixelA, pixelI); // E
        	uint16 p1 = average(pixelI, pixelJ);
        	d1[1] = average(pixelB, p1);
			
			s += 1;
        	s1 += 1;
        	d1 += 2;
		}
        ssrc += srowbytes;
		ddst += drowbytes * 2;
	}
}
/* Average filter (SdlEmu) */
