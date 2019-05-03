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

#include <portaudio.h>

#include "handy_sdl_main.h"
#include "handy_sound.h"

PaStream *apu_stream;

int handy_audio_init(void)
{
	int32_t err;
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

#ifdef HANDY_SDL_DEBUG
    printf("handy_audio_init - DEBUG\n");
#endif

	err = Pa_Initialize();
	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice();
	if (outputParameters.device == paNoDevice) 
	{
		printf("No sound output\n");
		gAudioEnabled = 0;
		return 0;
	}
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	err = Pa_OpenStream( &apu_stream, NULL, &outputParameters, HANDY_AUDIO_SAMPLE_FREQ, HANDY_AUDIO_BUFFER_SIZE, paNoFlag, NULL, NULL);
	err = Pa_StartStream( apu_stream );
	
	gAudioEnabled = 1;
    return 1;
}

void handy_audio_close()
{
	int32_t err;
	if (apu_stream)
	{
		err = Pa_CloseStream( apu_stream );
		err = Pa_Terminate();	
	}
}

void handy_audio_loop()
{
	mpLynx->Update();
	if (gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2 && gAudioEnabled)
	{
		uint32_t f = gAudioBufferPointer;
		gAudioBufferPointer = 0;	
		Pa_WriteStream( apu_stream, gAudioBuffer, f/4);
	}
}
