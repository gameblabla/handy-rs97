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
// handy_sdl_sound.cpp                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL sound source. It manages the sound functions for   //
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
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>

#include "handy_sdl_main.h"
#include "handy_sdl_sound.h"

#if defined(LIBAO)
#include <ao/ao.h>
ao_device *aodevice;
ao_sample_format aoformat;
#elif defined(PORTAUDIO)
#include <portaudio.h>
PaStream *apu_stream;
#elif defined(OSS_OUTPUT)
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>
static int32_t oss_audio_fd = -1;
#else
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>
SDL_mutex *sound_mutex;
SDL_cond *sound_cv;
#endif

/*  
    Name                :   handy_sdl_audio_callback
    Parameters          :   userdata (NULL, not used)
                            stream   (sample data)
                            len      (sampel length)
    Function            :   Our SDL adio callback/output function
    
    Uses                :   gAudioBuffer        (Lynx Audio Buffer)
                            gAudioBufferPointer (Filled size of gAudioBuffer)
    
    Information         :   Only when gAudioBufferPointer is equal or larger 
                            then len, then we want to output the audio.
    
                            gAudioBufferPointer increases everytime the 
                            mpLynx->Update() function is called. It is 
                            possible that gAudioBufferPointer exceeds the
                            number of the sample length. So we allways reset 
                            the gAudioBuffer and gAudioBufferPointer based 
                            upon the differences between len en gAudioBufferPointer
*/
#if !defined(PORTAUDIO) && !defined(LIBAO) && !defined(OSS_OUTPUT)
void handy_sdl_audio_callback(void *userdata, Uint8 *stream, int len)
{
    uint32_t length = len >> 1;
    Uint16 *dst = (Uint16 *)stream;
    Uint8 *src = (Uint8 *)gAudioBuffer;

    SDL_LockMutex(sound_mutex);

    // expand to pseudo-stereo because rzx50/a380 dingux doesn't support mono
    if( ( (int)gAudioBufferPointer >= (len/2)) && (gAudioBufferPointer != 0) && (!gSystemHalt) ) {
        while(length) {
            Uint16 sample = *src++;
            sample |= sample << 8;
            *dst++ = sample;
            length--;
        }
        memmove(gAudioBuffer, gAudioBuffer+(len/2), gAudioBufferPointer - (len/2));
        gAudioBufferPointer = gAudioBufferPointer - (len/2);
    }
    SDL_CondSignal(sound_cv);
    SDL_UnlockMutex(sound_mutex);
}
#endif

/*  
    Name                :   handy_sdl_audio_init
    Parameters          :   N/A
    Function            :   Initialisation of the audio using the SDL libary.

    Uses                :   N/A
    
    Information         :   This is our initalisation function for getting our
                            desired audio setup. Since the Atari Lynx has 8-bit
                            audio, no stereo and a output of 22050hz we use these
                            values to setup audio.
                            
                            Because of portability, our samples need to be a value
                            powered by two. During tests we found out that this is
                            the best value for constant sound updates in combination
                            with the sound quality.
*/
int handy_sdl_audio_init(void)
{
    SDL_AudioSpec     *desired;
    
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

#ifdef HANDY_SDL_DEBUG
    printf("handy_sdl_audio_init - DEBUG\n");
#endif

#ifdef LIBAO
	ao_initialize();
	memset(&aoformat, 0, sizeof(aoformat));
	
	aoformat.bits = 16;
	aoformat.channels = 2;
	aoformat.rate = HANDY_AUDIO_SAMPLE_FREQ;
	aoformat.byte_format = AO_FMT_NATIVE;
	
	aodevice = ao_open_live(ao_default_driver_id(), &aoformat, NULL); // Live output
	gAudioEnabled = 1;
#elif defined(PORTAUDIO)
	int32_t err;
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
	err = Pa_OpenStream( &apu_stream, NULL, &outputParameters, HANDY_AUDIO_SAMPLE_FREQ, 1024, paNoFlag, NULL, NULL);
	err = Pa_StartStream( apu_stream );
#elif defined(OSS_OUTPUT)
	uint32_t channels = 2;
	uint32_t format = AFMT_S16_LE;
	uint32_t tmp = HANDY_AUDIO_SAMPLE_FREQ;
	uint32_t err_ret;
	
	oss_audio_fd = open("/dev/dsp", O_WRONLY);
	if (oss_audio_fd < 0)
	{
		printf("Couldn't open /dev/dsp.\n");
		gAudioEnabled = 0;
		return 0;
	}
	
	err_ret = ioctl(oss_audio_fd, SNDCTL_DSP_SPEED,&tmp);
	err_ret = ioctl(oss_audio_fd, SNDCTL_DSP_CHANNELS, &channels);
	err_ret = ioctl(oss_audio_fd, SNDCTL_DSP_SETFMT, &format);
#else
    /* Allocate a desired SDL_AudioSpec */
    desired = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    /* Define our desired SDL audio output */
    desired->format     = AUDIO_S16SYS;                  // Unsigned 8-bit
    desired->channels   = 2;                         // Pseudo stereo
    desired->freq       = HANDY_AUDIO_SAMPLE_FREQ*2;   // Freq : 22050 (output is 44100) 
    desired->samples    = 1024;                       // Samples (power of two)
    desired->callback   = handy_sdl_audio_callback;  // Our audio callback
    desired->userdata   = NULL;                      // N/A

    /* Check if we can get our desired SDL audio output */
    if(SDL_OpenAudio(desired, NULL) < 0) {
        fprintf(stderr, "ERROR : Couldn't open audio: %s\n", SDL_GetError());
        return 0;  
    }

    free(desired);

    sound_mutex = SDL_CreateMutex();
    sound_cv = SDL_CreateCond();
    
    /* Enable SDL audio */
    SDL_PauseAudio(0);
#endif
	gAudioEnabled = 1;
    return 1;
}

void handy_sdl_close()
{
#ifdef PORTAUDIO
	int32_t err;
	err = Pa_CloseStream( apu_stream );
	err = Pa_Terminate();	
#elif defined(LIBAO)
	ao_close(aodevice);
	ao_shutdown();
#elif defined(OSS_OUTPUT)
	if (oss_audio_fd >= 0)
	{
		close(oss_audio_fd);
	}
#else
	SDL_PauseAudio(1);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif
}

void handy_sdl_sound_loop()
{
#if defined(PORTAUDIO) || defined(LIBAO) || defined(OSS_OUTPUT)
	mpLynx->Update();
	if(gAudioBufferPointer > 0 && gAudioEnabled)
	{
		uint32_t f = gAudioBufferPointer;
		gAudioBufferPointer = 0;	
		#ifdef LIBAO
		ao_play(aodevice, (char*)gAudioBuffer, f);
		#elif defined(OSS_OUTPUT)
		write(oss_audio_fd, gAudioBuffer, f );
		#else
		Pa_WriteStream( apu_stream, gAudioBuffer, f);
		#endif
	}
#else // Assuming SDL
	/* Quite honestly, this is very poor sound code and does not play well with Triple buffering.
	 * Portaudio and/or libao should be preferred instead. - Gameblabla*/
	// synchronize by sound samples
	SDL_LockMutex(sound_mutex);
	for(uint32_t loop=256;loop;loop--)
	{
#ifndef SDL_TRIPLEBUF
		extern int Throttle;
		if(Throttle) while(gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2) SDL_CondWait(sound_cv, sound_mutex);
#endif
		mpLynx->Update();
	}
	SDL_CondSignal(sound_cv);
	SDL_UnlockMutex(sound_mutex);
#endif

}
