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
// handy_sdl_main.cpp                                                       //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the main Handy/SDL source. It manages the main functions for     //
// emulating the Atari Lynx emulator using the SDL Library.                 //
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
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <unistd.h>
#include <dirent.h>
#include <SDL/SDL.h>

#include "handy_sdl_main.h"
#include "handy_sdl_graphics.h"
#include "handy_sdl_handling.h"
#include "handy_sound.h"
#include "gui/gui.h"

#include <sys/stat.h>
#include <sys/types.h>

/* SDL declarations */
SDL_Surface        *HandyBuffer, *mainSurface;
extern SDL_Surface* menuSurface;
#ifndef NOJOYSTICK
SDL_Joystick *joystick;
#endif
/* Handy declarations */
ULONG				*mpLynxBuffer;
CSystem				*mpLynx;
int                 mFrameSkip = 0;
int                 mpBpp;                    // Lynx rendering bpp

/* Handy/SDL declarations */
int                 LynxWidth;                // Lynx SDL screen width
int                 LynxHeight;              // Lynx SDL screen height
int              LynxFormat = MIKIE_PIXEL_FORMAT_16BPP_565;                // Lynx ROM format type
int              LynxRotate;                // Lynx ROM rotation type

int                 emulation = 0;


/*
    Name                :     handy_sdl_update
    Parameters          :     N/A
    Function            :   Update/Throttle function for Handy/SDL.

    Uses                :   N/A

    Information            :    This function is basicly the Update() function from
                            Handy WIN32 with minor tweaks for SDL. It is used for
                            basic throttle of the Handy core.
*/
inline int handy_sdl_update(void)
{
        // Throttling code
        //
        if(gSystemCycleCount > gThrottleNextCycleCheckpoint)
        {
            int overrun=gSystemCycleCount-gThrottleNextCycleCheckpoint;
            int nextstep=(((HANDY_SYSTEM_FREQ/HANDY_BASE_FPS)*gThrottleMaxPercentage)/100);

            // We've gone thru the checkpoint, so therefore the
            // we must have reached the next timer tick, if the
            // timer hasnt ticked then we've got here early. If
            // so then put the system to sleep by saying there
            // is no more idle work to be done in the idle loop

            if(gThrottleLastTimerCount==gTimerCount)
            {
                return 0;
            }
            
            //Set the next control point
            gThrottleNextCycleCheckpoint+=nextstep;

            // Set next timer checkpoint
            gThrottleLastTimerCount=gTimerCount;

            // Check if we've overstepped the speed limit
            if(overrun>nextstep)
            {
                // We've exceeded the next timepoint, going way too
                // fast (sprite drawing) so reschedule.
                return 0;
            }

        }

        return 1;

}


extern SDL_Surface* HandyBuffer;
extern int mRotation;
/* Required for Rotate Right games like Gauntlet and Klax as they have an extra 8 pixels on the top */
extern uint32_t Cut_Off_Y;

void Set_Rotation_Game()
{
    switch(mRotation) 
    {
        case CART_NO_ROTATE:
            LynxWidth = 160;
            LynxHeight = 102;
			Cut_Off_Y = 0;
            break;
        case CART_ROTATE_LEFT:
            LynxWidth = 102;
            LynxHeight = 160;
			Cut_Off_Y = 8;
        break;
        case CART_ROTATE_RIGHT:
            LynxWidth = 102;
            LynxHeight = 160;
			Cut_Off_Y = 0;
		break;
    }
    
    /* Don't clear the RGB surface as it will make it crash. Perhaps SDL does it on its own ??? */
    /* Also height needs to be 16 pixels bigger for vertical games as those will need cropping. */
    /* 8 pixels will be cropped and 8 more pixels are needed to avoid overflows. */
	HandyBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, LynxWidth, 176, 16, 0, 0, 0, 0);
}

/*
    Name                :     handy_sdl_rom_info
    Parameters          :     N/A
    Function            :   Game Image information function for Handy/SDL

    Uses                :   N/A

    Information            :    Basic function for getting information of the
                            Atari Lynx game image and for setting up the
                            Handy core concerning rotation.
*/
void handy_sdl_rom_info(void)
{
    printf("Atari Lynx ROM Information\n");

    /* Retrieving Game Image information */
    printf("Cartname      : %s\n"   , mpLynx->CartGetName()         );
    printf("ROM Size      : %d kb\n", (int)mpLynx->CartSize()      );
    printf("Manufacturer  : %s\n"   , mpLynx->CartGetManufacturer() );

	Set_Rotation_Game();

    /* Retrieving Game Image Rotatation */
    printf("Lynx Rotation : ");
    switch(mpLynx->CartGetRotate())
    {
        case CART_NO_ROTATE:
            LynxRotate = MIKIE_NO_ROTATE;
            printf("NO\n");
            break;
        case CART_ROTATE_LEFT:
            LynxRotate = MIKIE_ROTATE_L;
            printf("LEFT\n");
            break;
        case CART_ROTATE_RIGHT:
            LynxRotate = MIKIE_ROTATE_R;
            printf("RIGHT\n");
            break;
        default:
            // Allright, this shouldn't be necassary. But in case the user is using a
            // bad dump, we use the default rotation as in no rotation.
            LynxRotate = MIKIE_NO_ROTATE;
            printf("NO (forced)\n");
            break;
    }

    /* Retrieving Game Image type */
    printf("ROM Type      : ");
    switch(mpLynx->mFileType)
    {
        case HANDY_FILETYPE_HOMEBREW:
            printf("Homebrew\n");
            break;
        case HANDY_FILETYPE_LNX:
            printf("Commercial and/or .LNX-format\n");
            break;
        case HANDY_FILETYPE_SNAPSHOT:
            printf("Snapshot\n");
            break;
        default:
            // Allright, this shouldn't be necessary, but just in case.
            printf("Unknown format!\n");
            exit(EXIT_FAILURE);
            break;
    }
}

void handy_sdl_quit(void)
{
	extern int done;
    emulation   = -1;
    done = 1;
}

static void Cleanup_mess(void)
{
    // Disable audio and set emulation to pause, then quit :)
    handy_audio_close();
    
	if (HandyBuffer) SDL_FreeSurface(HandyBuffer);
	if (mainSurface) SDL_FreeSurface(mainSurface);
	if (menuSurface) SDL_FreeSurface(menuSurface);

    // Close SDL Subsystems
    SDL_QuitSubSystem(SDL_INIT_VIDEO
    #ifndef NOJOYSTICK
    |SDL_INIT_JOYSTICK
    #endif
    );
    SDL_Quit();
    
	if (mpLynx) delete mpLynx;
}


char rom_name_with_no_ext[128]; // rom name with no extension, used for savestates

void handy_sdl_core_init(char *romname)
{
    int i;

    // strip rom name from full path and cut off the extension
    for(i = strlen(romname) - 1; i >= 0; i--) {
        if(romname[i] == '/' || i == 0) { 
            memcpy((void *)rom_name_with_no_ext, (void *)(romname + i + (i?1:0)), strlen(romname) - i);
            rom_name_with_no_ext[strlen(rom_name_with_no_ext)-4] = 0; // cut off extension.
            break;
        }
    }
    
    // DEBUG
    //printf("Naked name: %s\n", (char *)&rom_name_with_no_ext);

    printf("Initialising Handy Core...    ");
	mpLynx = new CSystem(romname);
    printf("[DONE]\n\n");

    // DEBUG
    //printf("Rom full path: %s\n", romname);
    //printf("Bios full path: %s\n", bios_path_and_name);

    // Query Rom Image information
    handy_sdl_rom_info();
}

/*
    Reinit routine used to reload roms on the fly
*/
void handy_sdl_core_reinit(char *romname)
{
    if (mpLynx) delete mpLynx;
    handy_sdl_core_init(romname);
    handy_sdl_video_init(mpBpp);
}

int Throttle = 1;  // Throttle to 60FPS

int main(int argc, char *argv[])
{
    int i;
    SDL_Event handy_sdl_event;
    char load_filename[512];
    char romname[512];
    
    // get bios path
    getcwd(load_filename, 512);

    gAudioEnabled = FALSE; // Disable audio by default for Zipit

    // Default output
    printf("Handy GCC/SDL Portable Atari Lynx Emulator %s\n", HANDY_SDL_VERSION);
    printf("Based upon %s by Keith Wilkins\n", HANDY_VERSION);
    printf("Written by SDLEmu Team, additions by Pierre Doucet\n");
    printf("Contact: http://sdlemu.ngemu.com | shalafi@xs4all.nl\n\n");

    // If no argument given - call filebrowser
    // As SDL is not initialized yet, gui_LoadFile calls gui_video_early_init()
    if (argc < 2) 
    {
		printf("We need a ROM to load\n");
		return 1;
    }
    else
    {
		snprintf(romname, sizeof(romname), "%s", argv[1]);
	}

    for ( i=0; (i < argc || argv[i] != NULL ); i++ )
    {
        if (!strcmp(argv[i], "-sound"))         gAudioEnabled = TRUE;
        if (!strcmp(argv[i], "-nosound"))         gAudioEnabled = FALSE;
    }
    gAudioEnabled = TRUE;

    // Initalising SDL for Audio and Video support
    printf("Initialising SDL...           ");
    if (SDL_Init(SDL_INIT_VIDEO
    #ifndef NOJOYSTICK
    | SDL_INIT_JOYSTICK
    #endif
    ) < 0) {
        fprintf(stderr, "FAILED : Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    #ifndef NOJOYSTICK
	if(SDL_NumJoysticks() > 0)
		joystick = SDL_JoystickOpen(0);
	#endif
    printf("[DONE]\n");

    // Primary initalise of Handy - should be called AFTER SDL_Init() but BEFORE handy_sdl_video_setup()
    handy_sdl_core_init(romname);

    // Initialise Handy/SDL video 
    if(!Handy_Init_Video())
    {
		SDL_Quit();
        return 1;
    }

    // Initialise Handy/SDL audio
    printf("\nInitialising SDL Audio...     ");
    if(handy_audio_init())
    {
        gAudioEnabled = TRUE;
    }
    printf("[DONE]\n");

    // Setup of Handy Core video
    handy_sdl_video_init(mpBpp);

    // Init gui (move to some other place later)
    gui_Init();

    printf("Starting Lynx Emulation...\n");
    while(!emulation)
    {
        // Initialise Handy button events
        int OldKeyMask, KeyMask = mpLynx->GetButtonData();
        OldKeyMask = KeyMask;
        
        #ifndef NOJOYSTICK
		SDL_JoystickUpdate();
		#endif
        
        KeyMask = Joystick_Down(KeyMask, handy_sdl_event);

        // Checking if we had SDL handling events and then we'll update the Handy button events.
        if (OldKeyMask != KeyMask)
            mpLynx->SetButtonData(KeyMask);

        // Update TimerCount
        gTimerCount++;
        
        while( handy_sdl_update() )
        {
            if(!gSystemHalt)
            {
				handy_audio_loop();
            }
            else
            {
				gTimerCount++;
            }
        }
    }
    
    Cleanup_mess();
    return 0;
}
