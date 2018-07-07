Handy/SDL RS97
===============

Here's an improved version of Handy for the RS-97 based on Handy libretro and the Dingoo port of Handy.
The bios specific code got stripped out for mostly legal and conveniance reasons.
The other reason was due to the fact that it tended to make things crash...
The HLE Bios as part of Handy libretro is pretty good anyway and it's not needed.

Unlike the libretro version though, this one still runs at 8-bits 22050Hz.
I couldn't figure out how to make it work properly at 44100Hz 16-bits with the existing sound code
so i had to do this. Hopefully someone will pick this up in the future.

This emulator was licensed under the GPLv2 by the emulator's author.
