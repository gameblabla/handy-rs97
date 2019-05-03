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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>

#include <ao/ao.h>

#include "handy_sdl_main.h"
#include "handy_sound.h"

ao_device *aodevice;
ao_sample_format aoformat;

int handy_audio_init(void)
{
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

#ifdef HANDY_SDL_DEBUG
    printf("handy_audio_init - DEBUG\n");
#endif

	ao_initialize();
	memset(&aoformat, 0, sizeof(aoformat));
	
	aoformat.bits = 16;
	aoformat.channels = 2;
	aoformat.rate = HANDY_AUDIO_SAMPLE_FREQ;
	aoformat.byte_format = AO_FMT_NATIVE;
	
	aodevice = ao_open_live(ao_default_driver_id(), &aoformat, NULL); // Live output
	
	gAudioEnabled = 1;

    return 1;
}

void handy_audio_close()
{
	if (aodevice)
	{
		ao_close(aodevice);
		ao_shutdown();
	}
}

void handy_audio_loop()
{
	mpLynx->Update();
	if (gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2 && gAudioEnabled)
	{
		uint32_t f = gAudioBufferPointer;
		gAudioBufferPointer = 0;	
		ao_play(aodevice, (char*)gAudioBuffer, f/4);
	}
}
