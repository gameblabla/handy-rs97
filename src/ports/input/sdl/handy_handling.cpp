//
// Copyright (c) 2004 SDLemu Team
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy/SDL - An Atari Lynx Emulator                 //
//                             Copyright (c) 2005                           //
//                                SDLemu Team                               //
//                                                                          //
//                          Based upon Handy v0.90 WIN32                    // 
//                            Copyright (c) 1996,1997                       //
//                                  K. Wilkins                              //
//////////////////////////////////////////////////////////////////////////////
// handy_sdl_graphics.cpp                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL handling. It manages the handling functions        //
// of the keyboard and/or joypad for emulating the Atari Lynx emulator      //
// using the SDL Library.                                                     //
//                                                                          //
//    N. Wagenaar                                                           //
// December 2005                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// December 2005 :                                                          //
//  Since the 14th of April, the WIN32 of Handy (written by Keith Wilkins)  //
//  Handy has become OpenSource. Handy/SDL v0.82 R1 was based upon the old  //
//  v0.82 sources and was released closed source.                           //
//                                                                          //
//  Because of this event, the new Handy/SDL will be released as OpenSource //
//  but is rewritten from scratch because of lost sources (tm). The SDLemu  //
//  team has tried to bring Handy/SDL v0.1 with al the functions from the   //
//  closed source version.                                                  //
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>

#include "handy_sdl_main.h"
#include "handy_sdl_handling.h"

extern int Invert;
extern SDL_Joystick* joystick;

//	SDL_JoystickUpdate();

// map keys differently for zipit
uint32_t BT_A = SDLK_LCTRL;
uint32_t BT_B = SDLK_LALT;
#define BT_LEFT     SDLK_LEFT
#define BT_RIGHT    SDLK_RIGHT
#define BT_UP       SDLK_UP
#define BT_DOWN     SDLK_DOWN
#define BT_PAUSE    SDLK_RETURN      // START
//#define BT_A        SDLK_LCTRL       // A
//#define BT_B        SDLK_LALT        // B
#define BT_OPT1     SDLK_LSHIFT      // Y
#define BT_OPT2     SDLK_SPACE       // X
#define BT_OFF      SDLK_ESCAPE      // SELECT

uint32_t Joystick_Down(uint32_t mask)
{
    int16_t x_move = 0, y_move = 0;
    if (joystick) 
    {
        x_move = SDL_JoystickGetAxis(joystick, 0);
        y_move = SDL_JoystickGetAxis(joystick, 1);

		switch (Invert)
		{
			case 0:
				if(x_move > 500)
					mask |= BUTTON_RIGHT;

				if(x_move < -500)
					mask |= BUTTON_LEFT;

				if(y_move > 500)
					mask |= BUTTON_DOWN;

				if(y_move < -500)
					mask |= BUTTON_UP;
			break;
			case 1:
				if(x_move > 500)
					mask |= BUTTON_DOWN;
				if(x_move < -500)
					mask |= BUTTON_UP;

				 if(y_move > 500)
					mask |= BUTTON_LEFT;

				if(y_move < -500)
					mask |= BUTTON_RIGHT;
			break;
			
			/* KLAX */
			case 2:
				if (x_move > 500)
				mask |= BUTTON_UP;
				if (x_move < -500)
				mask |= BUTTON_DOWN;

				if (y_move > 500)
				mask |= BUTTON_RIGHT;

				if (y_move < -500)
				mask |= BUTTON_LEFT;
			break;
		}

		if(SDL_JoystickGetButton(joystick, 1) == SDL_PRESSED)
			mask |= BUTTON_A;
		else if(SDL_JoystickGetButton(joystick, 1) == SDL_RELEASED)
			mask &= ~BUTTON_A;
			
		if(SDL_JoystickGetButton(joystick, 2) == SDL_PRESSED)
			mask |= BUTTON_B;
		else if(SDL_JoystickGetButton(joystick, 2) == SDL_RELEASED)
			mask &= ~BUTTON_B;
			
		if(SDL_JoystickGetButton(joystick, 0) == SDL_PRESSED)
			mask |= BUTTON_OPT1;
		else if(SDL_JoystickGetButton(joystick, 0) == SDL_RELEASED)
			mask &= ~BUTTON_OPT1;
			
		if(SDL_JoystickGetButton(joystick, 3) == SDL_PRESSED)
			mask |= BUTTON_OPT2;
		else if(SDL_JoystickGetButton(joystick, 3) == SDL_RELEASED)
			mask &= ~BUTTON_OPT2;		
    }
	return mask;
}


int  handy_sdl_on_key_down(SDL_KeyboardEvent key, int mask)
{
    if(key.keysym.sym == BT_B) return mask |= BUTTON_B;
    if(key.keysym.sym == BT_A) return mask |= BUTTON_A;

	switch(key.keysym.sym) 
	{
			case BT_LEFT: {  // Lynx LEFT
				switch(Invert)
				{
					case 0:
						mask|=BUTTON_LEFT;
					break;
					case 1:
						mask|=BUTTON_UP;
					break;
					case 2:
						mask|=BUTTON_DOWN;
					break;
				}
				break;
			}
			case BT_RIGHT: { // Lynx RIGHT
				switch(Invert)
				{
					case 0:
						mask|=BUTTON_RIGHT;
					break;
					case 1:
						mask|=BUTTON_DOWN;
					break;
					case 2:
						mask|=BUTTON_UP;
					break;
				}
				break;
			}

			case BT_UP: { // Lynx UP
				switch(Invert)
				{
					case 0:
						mask|=BUTTON_UP;
					break;
					case 1:
						mask|=BUTTON_RIGHT;
					break;
					case 2:
						mask|=BUTTON_LEFT;
					break;
				}
				break;
			}

			case BT_DOWN: { // Lynx DOWN
				switch(Invert)
				{
					case 0:
						mask|=BUTTON_DOWN;
					break;
					case 1:
						mask|=BUTTON_LEFT;
					break;
					case 2:
						mask|=BUTTON_RIGHT;
					break;
				}
				break;
			}

			case BT_PAUSE: { // Lynx PAUSE
				mask|=BUTTON_PAUSE;
				break;
			}

			case BT_OPT1: { // Lynx Option 1
				mask|=BUTTON_OPT1;
				break;
			}

			case BT_OPT2: { // Lynx Option 2
				mask|=BUTTON_OPT2;
				break;
			}

			default: {
				break;
			}

	}

    return mask;

}

int  handy_sdl_on_key_up(SDL_KeyboardEvent key, int mask)
{
    if(key.keysym.sym == BT_B) return mask &= ~BUTTON_B;
    if(key.keysym.sym == BT_A) return mask &= ~BUTTON_A;

	switch(key.keysym.sym) 
	{
			case BT_LEFT: {  // Lynx LEFT
				switch(Invert)
				{
					case 0:
						mask&= ~BUTTON_LEFT;
					break;
					case 1:
						mask&= ~BUTTON_UP;
					break;
					case 2:
						mask&= ~BUTTON_DOWN;
					break;
				}
				break;
			}
			case BT_RIGHT: { // Lynx RIGHT
				switch(Invert)
				{
					case 0:
						mask&= ~BUTTON_RIGHT;
					break;
					case 1:
						mask&= ~BUTTON_DOWN;
					break;
					case 2:
						mask&= ~BUTTON_UP;
					break;
				}
				break;
			}

			case BT_UP: { // Lynx UP
				switch(Invert)
				{
					case 0:
						mask&= ~BUTTON_UP;
					break;
					case 1:
						mask&= ~BUTTON_RIGHT;
					break;
					case 2:
						mask&= ~BUTTON_LEFT;
					break;
				}
				break;
			}

			case BT_DOWN: { // Lynx DOWN
				switch(Invert)
				{
					case 0:
						mask&= ~BUTTON_DOWN;
					break;
					case 1:
						mask&= ~BUTTON_LEFT;
					break;
					case 2:
						mask&= ~BUTTON_RIGHT;
					break;
				}
				break;
			}

			case BT_PAUSE: { // Lynx PAUSE
				mask&= ~BUTTON_PAUSE;
				break;
			}

			case BT_OPT1: { // Lynx Option 1
				mask&= ~BUTTON_OPT1;
				break;
			}

			case BT_OPT2: { // Lynx Option 2
				mask&= ~BUTTON_OPT2;
				break;
			}

			default: {
				break;
			}

	}
	
    return mask;
}
