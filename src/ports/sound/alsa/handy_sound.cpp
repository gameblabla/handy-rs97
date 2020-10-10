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

#include <alsa/asoundlib.h>

#include "handy_sdl_main.h"
#include "handy_sound.h"

static snd_pcm_t *handle;

int handy_audio_init(void)
{
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

#ifdef HANDY_SDL_DEBUG
    printf("handy_audio_init - DEBUG\n");
#endif

	snd_pcm_hw_params_t *params;
	uint32_t val;
	int32_t dir = -1;
	snd_pcm_uframes_t frames;
	
	/* Open PCM device for playback. */
	int32_t rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
		
	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
	{
		fprintf(stderr, "unable to open PCM device: %s\n", snd_strerror(rc));
		return 1;
	}
	
	rc = snd_config_update_free_global();
	
#ifdef NONBLOCKING_AUDIO
	snd_pcm_nonblock(handle, 1);
#else
	snd_pcm_nonblock(handle, 0);
#endif
	
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	rc = snd_pcm_hw_params_any(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_any %s\n", snd_strerror(rc));
		return 1;
	}

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_access %s\n", snd_strerror(rc));
		return 1;
	}

	/* Signed 16-bit little-endian format */
	rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_format %s\n", snd_strerror(rc));
		return 1;
	}

	/* Two channels (stereo) */
	rc = snd_pcm_hw_params_set_channels(handle, params, 2);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_channels %s\n", snd_strerror(rc));
		return 1;
	}
	
	val = HANDY_AUDIO_SAMPLE_FREQ;
	rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_rate_near %s\n", snd_strerror(rc));
		return 1;
	}

	frames = HANDY_AUDIO_BUFFER_SIZE;
	rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return 1;
	}
	
	frames *= 4;
	rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &frames);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return 1;
	}

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Unable to set hw parameters: %s\n", snd_strerror(rc));
		return 1;
	}
	
	gAudioEnabled = 1;
    return 1;
}

void handy_audio_close()
{
	if (handle)
	{
		snd_pcm_drop(handle);
		snd_pcm_close(handle);
		snd_config_update_free_global();
	}
}

void handy_audio_loop()
{
	mpLynx->Update();
	if (gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2 && gAudioEnabled)
	{
		uint32_t f = gAudioBufferPointer;
		gAudioBufferPointer = 0;	
		long ret, len = f / 4;
		ret = snd_pcm_writei(handle, gAudioBuffer, len);
		while(ret != len) 
		{
			if (ret < 0) 
			{
				snd_pcm_prepare( handle );
			}
			else 
			{
				len -= ret;
			}
			ret = snd_pcm_writei(handle, gAudioBuffer, len);
		}
	}
}
