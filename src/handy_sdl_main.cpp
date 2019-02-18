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
#include "handy_sdl_sound.h"
#include "handy_sdl_usage.h"
#ifdef DINGUX
#include "gui/gui.h"
#endif

#include <sys/stat.h>
#include <sys/types.h>

/* SDL declarations */
SDL_Surface        *HandyBuffer;             // Our Handy/SDL display buffer
SDL_Surface        *mainSurface;             // Our Handy/SDL primary display
SDL_Joystick *joystick;
extern uint32_t Joystick_Down(uint32_t mask);

/* Handy declarations */
ULONG            *mpLynxBuffer;
CSystem         *mpLynx;
int                 mFrameSkip = 0;
int                 mpBpp;                    // Lynx rendering bpp

/* Handy/SDL declarations */
int                 LynxWidth;                // Lynx SDL screen width
int                 LynxHeight;              // Lynx SDL screen height
int                LynxScale = 1;            // Factor to scale the display
int                LynxLCD = 1;            // Emulate LCD Display
int              LynxFormat;                // Lynx ROM format type
int              LynxRotate;                // Lynx ROM rotation type
#ifndef DINGUX
uint32_t          overlay_format = SDL_YV12_OVERLAY; // YUV Overlay format
#endif

int                 emulation = 0;
Uint8          *delta;
/*
    Handy/SDL Rendering output

    1 = SDL rendering
    2 = OpenGL rendering
    3 = YUV Overlay rendering

    Default = 1 (SDL)
*/
int                rendertype = 1;

/*
    Handy/SDL Scaling/Scanline routine

    1 = SDLEmu v1 (compatible with al SDL versions)
    2 = SDLEmu v2 (faster but might break in future SDL versions or on certain platforms)
    3 = Pierre Doucet v1 (compatible but possiby slow)

    Default = 1 (SDLEmu v1)
*/

int                stype = 1;                // Scaling/Scanline routine.


/*
    Handy/SDL Filter selection

    1 = SDLEmu v1 (compatible with al SDL versions)
    2 = SDLEmu v2 (faster but might break in future SDL versions or on certain platforms)
    3 = Pierre Doucet v1 (compatible but possiby slow)

    Default = 1 (SDLEmu v1)
*/

int                filter = 6;                // Scaling/Scanline routine.


/*
    Name                :     handy_sdl_update
    Parameters          :     N/A
    Function            :   Update/Throttle function for Handy/SDL.

    Uses                :   N/A

    Information            :    This function is basicly the Update() function from
                            Handy WIN32 with minor tweaks for SDL. It is used for
                            basic throttle of the Handy core.
*/
inline    int handy_sdl_update(void)
{


        // Throttling code
        //
        if(gSystemCycleCount>gThrottleNextCycleCheckpoint)
        {
            static int limiter=0;
            static int flipflop=0;
            int overrun=gSystemCycleCount-gThrottleNextCycleCheckpoint;
            int nextstep=(((HANDY_SYSTEM_FREQ/HANDY_BASE_FPS)*gThrottleMaxPercentage)/100);

            // We've gone thru the checkpoint, so therefore the
            // we must have reached the next timer tick, if the
            // timer hasnt ticked then we've got here early. If
            // so then put the system to sleep by saying there
            // is no more idle work to be done in the idle loop

            if(gThrottleLastTimerCount==gTimerCount)
            {
                // All we know is that we got here earlier than expected as the
                // counter has not yet rolled over
                if(limiter<0) limiter=0; else limiter++;
                if(limiter>40 && mFrameSkip>0)
                {
                    mFrameSkip--;
                    limiter=0;
                }
                flipflop=1;
                return 0;
            }

            // Frame Skip adjustment
            if(!flipflop)
            {
                if(limiter>0) limiter=0; else limiter--;
                if(limiter<-7 && mFrameSkip<10)
                {
                    mFrameSkip++;
                    limiter=0;
                }
            }

            flipflop=0;

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
    // Disable audio and set emulation to pause, then quit :)
    handy_sdl_close();
    emulation   = -1;

#ifndef DINGUX
    //Remove YUV Overlay
    if ( rendertype == 3 )
        handy_sdl_video_close();
#endif

    //Let is give some free memory
	if (mpLynxBuffer != NULL) free(mpLynxBuffer);

	if (HandyBuffer != NULL) SDL_FreeSurface(HandyBuffer);
	if (mainSurface != NULL) SDL_FreeSurface(mainSurface);

    // Close SDL Subsystems
    SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
    SDL_Quit();
    exit(EXIT_SUCCESS);

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
    delete mpLynx;
    handy_sdl_core_init(romname);
    handy_sdl_video_init(mpBpp);
}

#undef main // necessary for win32 compile

int Throttle = 1;  // Throttle to 60FPS

int main(int argc, char *argv[])
{
    int       i;
    int       frameskip  = 0;   // Frameskip
    SDL_Event handy_sdl_event;
    uint32_t    handy_sdl_start_time;
    uint32_t    handy_sdl_this_time;
    int       framecounter = 0; // FPS Counter
    int       Autoskip = 0;     // Autoskip
    int       Skipped = 0;
    int       Fullscreen = 0;
    float fps_counter;
#ifdef DINGUX
    int       bpp = 16;        // dingux has 16 hardcoded
#else
    int       bpp = 0;         // BPP -> 8,16 or 32. 0 = autodetect (default)
    int       fsaa = 0;        // OpenGL FSAA (default off)
    int       accel = 1;       // OpenGL Hardware accel (default on)
    int       sync  = 0;       // OpenGL VSYNC (default off)
    int       overlay = 1;     // YUV Overlay format
    char      overlaytype[4];  // Overlay Format
#endif
    char load_filename[512];
    char romname[512];
    
    printf("STARTY EMULATOR\n");

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
    if (argc < 2) {
#ifdef DINGUX
        if(gui_LoadFile(load_filename))  {
            snprintf(romname, sizeof(romname), "%s", load_filename);
        } else {
            handy_sdl_usage();
            exit(EXIT_FAILURE);
        }
#else
        handy_sdl_usage();
        exit(EXIT_FAILURE);
#endif
    }
    else
    {
		snprintf(romname, sizeof(romname), "%s", argv[1]);
	}

    for ( i=0; (i < argc || argv[i] != NULL ); i++ )
    {
        if (!strcmp(argv[i], "-throttle"))     Throttle = 1;
        if (!strcmp(argv[i], "-nothrottle"))     Throttle = 0;
#ifndef DINGUX
        if (!strcmp(argv[i], "-autoskip"))     Autoskip = 1;
        if (!strcmp(argv[i], "-noautoskip"))     Autoskip = 0;
        if (!strcmp(argv[i], "-fps"))             framecounter = 1;
        if (!strcmp(argv[i], "-nofps"))         framecounter = 0;
#endif
        if (!strcmp(argv[i], "-sound"))         gAudioEnabled = TRUE;
        if (!strcmp(argv[i], "-nosound"))         gAudioEnabled = FALSE;
#ifndef DINGUX
        if (!strcmp(argv[i], "-fullscreen"))    Fullscreen = 1;
        if (!strcmp(argv[i], "-nofullscreen"))    Fullscreen = 0;
        if (!strcmp(argv[i], "-fsaa"))            fsaa = 1;
        if (!strcmp(argv[i], "-nofsaa"))        fsaa = 0;
        if (!strcmp(argv[i], "-accel"))        accel = 1;
        if (!strcmp(argv[i], "-noaccel"))        accel = 0;
        if (!strcmp(argv[i], "-sync"))            sync = 1;
        if (!strcmp(argv[i], "-nosync"))        sync = 0;
        if (!strcmp(argv[i], "-2")) LynxScale = 2;
        if (!strcmp(argv[i], "-3")) LynxScale = 3;
        if (!strcmp(argv[i], "-4")) LynxScale = 4;
        if (!strcmp(argv[i], "-lcd")) LynxLCD = 1;
        if (!strcmp(argv[i], "-nolcd")) LynxLCD = 0;
#endif
        if (!strcmp(argv[i], "-frameskip"))
        {
            frameskip = atoi(argv[++i]);
            if ( frameskip > 9 )
                frameskip = 9;
        }
#ifndef DINGUX
        if (!strcmp(argv[i], "-bpp"))
        {
            bpp = atoi(argv[++i]);
            if ( (bpp != 0) && (bpp != 8) && (bpp != 15) && (bpp != 16) && (bpp != 24) && (bpp != 32) )
            {
                bpp = 0;
            }
        }
        if (!strcmp(argv[i], "-rtype"))
        {
            rendertype = atoi(argv[++i]);
            if ( (rendertype != 1) && (rendertype != 2) && (rendertype != 3))
            {
                rendertype = 1;
            }
        }
        if (!strcmp(argv[i], "-stype"))
        {
            stype = atoi(argv[++i]);
            if ( (stype != 1) && (stype != 2) && (stype != 3))
            {
                stype = 1;
            }
        }

        if (!strcmp(argv[i], "-filter"))
        {
            filter = atoi(argv[++i]);
            // Check if the filter number is larger then 1 and not more then 10.
            if ( (filter <= 10) && (filter >= 1) )
            {
                rendertype =  1;  // Filter type only works with SDL rendering
                LynxScale  =  2;  // Maximum size is 2 times
                bpp        = 16;  // Maximum BPP is 16.
            }
            // Otherwise disable the filter
            else
            {
                filter = 0;
            }
        }

        if (!strcmp(argv[i], "-format"))
        {
            overlay = atoi(argv[++i]);
            if ( ( overlay <= 5 ) && (overlay >= 1) )
            {
                switch(overlay) {
                    case 1: 
                        overlay_format = SDL_YV12_OVERLAY;
                        strcpy( overlaytype, "YV12" );
                        break;
                    case 2:
                        overlay_format = SDL_IYUV_OVERLAY;
                        strcpy( overlaytype, "IYUV" );
                        break;
                    case 3:
                        overlay_format = SDL_YUY2_OVERLAY;
                        strcpy( overlaytype, "YUY2" );
                        break;
                    case 4:
                        overlay_format = SDL_UYVY_OVERLAY;
                        strcpy( overlaytype, "UYVY" );
                        break;
                    case 5:
                        overlay_format = SDL_YVYU_OVERLAY;
                        strcpy( overlaytype, "YVYU" );
                        break;
                }
            }    
            else
            {
                overlay_format = SDL_YV12_OVERLAY;
                strcpy( overlaytype, "YV12" );
                
            }
            printf("Using YUV Overlay format: %s\n",overlaytype);
        }
#endif
    }
    gAudioEnabled = TRUE;

    // Initalising SDL for Audio and Video support
    printf("Initialising SDL...           ");
    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "FAILED : Unable to init SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
	if(SDL_NumJoysticks()>0)
		joystick = SDL_JoystickOpen(0);
    printf("[DONE]\n");

    // Primary initalise of Handy - should be called AFTER SDL_Init() but BEFORE handy_sdl_video_setup()
    handy_sdl_core_init(romname);

    // Initialise Handy/SDL video 
#ifndef DINGUX
    if(!handy_sdl_video_setup(rendertype, fsaa, Fullscreen, bpp, LynxScale, accel, sync))
#else
    if(!handy_sdl_video_setup(1, 0, 0, bpp, LynxScale, 0, 0))
#endif
    {
        return 0;
    }

    // Initialise Handy/SDL audio
    printf("\nInitialising SDL Audio...     ");
    if(handy_sdl_audio_init())
    {
        gAudioEnabled = TRUE;
    }
    printf("[DONE]\n");


    // Setup of Handy Core video
    handy_sdl_video_init(mpBpp);

    // Init gui (move to some other place later)
#ifdef DINGUX
    gui_Init();
#endif

    handy_sdl_start_time = SDL_GetTicks();

    printf("Starting Lynx Emulation...\n");
    while(!emulation)
    {
        // Initialise Handy button events
        int OldKeyMask, KeyMask = mpLynx->GetButtonData();
        OldKeyMask = KeyMask;
        
		SDL_JoystickUpdate();
		
		KeyMask = Joystick_Down(KeyMask);

        // Getting events for keyboard and/or joypad handling
        while(SDL_PollEvent(&handy_sdl_event))
        {
            switch(handy_sdl_event.type)
            {
                case SDL_KEYUP:
                    KeyMask = handy_sdl_on_key_up(handy_sdl_event.key, KeyMask);
                    break;
                case SDL_KEYDOWN:
                    #ifdef DINGUX
                    if(handy_sdl_event.key.keysym.sym == SDLK_BACKSPACE) {
                        //filter = (filter + 1) % 11;
                        if(filter != 6) filter = 6; else filter = 0;
                        SDL_FillRect(mainSurface,NULL,SDL_MapRGBA(mainSurface->format, 0, 0, 0, 255));
                        SDL_Flip(mainSurface);
                        SDL_FillRect(mainSurface,NULL,SDL_MapRGBA(mainSurface->format, 0, 0, 0, 255));
                        SDL_Flip(mainSurface);
                        break;
                    }
                    if(handy_sdl_event.key.keysym.sym == SDLK_ESCAPE || handy_sdl_event.key.keysym.sym == SDLK_3) {
                        gui_Run();
                        KeyMask = 0;
                        break;
                    }
                    #endif
                    KeyMask = handy_sdl_on_key_down(handy_sdl_event.key, KeyMask);
                    break;
                default:
                    KeyMask = 0;
                    break;
            }
        }

        // Checking if we had SDL handling events and then we'll update the Handy button events.
        if (OldKeyMask != KeyMask)
            mpLynx->SetButtonData(KeyMask);

        // Update TimerCount
        gTimerCount++;

		uint32_t start;

        while( handy_sdl_update() )
        {
            if(!gSystemHalt)
            {
				handy_sdl_sound_loop();
            }
            else
            {
#ifdef HANDY_SDL_DEBUG
                    printf("gSystemHalt : %d\n", gSystemHalt);
#endif
                    gTimerCount++;
            }
        }

        // Update screen manually
        //handy_sdl_display_callback(NULL);
#ifdef DINGUX
        gui_CountFPS(); // count fps my way :)
#endif

        handy_sdl_this_time = SDL_GetTicks();

        fps_counter = (((float)gTimerCount/(handy_sdl_this_time-handy_sdl_start_time))*1000.0);
#ifdef HANDY_SDL_DEBUG
        printf("fps_counter : %f\n", fps_counter);
#endif

        // not needed since we are synchronizing by sound
        //if( (Throttle) && (fps_counter > 59.99) ) SDL_Delay( (uint32_t)fps_counter );

#ifndef DINGUX
        if(Autoskip)
        {
            if(fps_counter > 60)
            {
                frameskip--;
                   Skipped = frameskip;
            }
            else
            {
                if(fps_counter < 60)
                {
                       Skipped++;
                       frameskip++;
                   }
            }
        }

        if ( framecounter )
        {

            if ( handy_sdl_this_time != handy_sdl_start_time )
            {
                static char buffer[256];

                sprintf (buffer, "Handy %f ", fps_counter);
                strcat( buffer, "FPS");
                SDL_WM_SetCaption( buffer , "HANDY" );
            }
        }
#endif
    }

    return 0;
}
