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

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>

#include "handy_sdl_main.h"
#include "handy_sound.h"

static int32_t oss_audio_fd = -1;

int handy_sdl_audio_init(void)
{
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

	uint32_t channels = 2;
	uint32_t format = AFMT_S16_LE;
	uint32_t tmp = HANDY_AUDIO_SAMPLE_FREQ;
	uint32_t err_ret;
	
	printf("\nOpening DSP device\n");
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
	
	gAudioEnabled = 1;
    return 1;
}

void handy_sdl_close()
{
	if (oss_audio_fd >= 0)
	{
		close(oss_audio_fd);
	}
}

void handy_sdl_sound_loop()
{
	mpLynx->Update();
	if (gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2 && gAudioEnabled)
	{
		uint32_t f = gAudioBufferPointer;
		gAudioBufferPointer = 0;	
		write(oss_audio_fd, gAudioBuffer, f );
	}
}
