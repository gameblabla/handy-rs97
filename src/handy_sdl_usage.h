void handy_sdl_usage()
{
printf("Usage: ./handy_sdl romfile [options]\n");
printf("\n");
printf(" Global Handy/SDL options:\n");
printf("\n");
printf(" romfile              : Filename of the romfile. It can have several\n");
printf("                        extensions (.lnx/.o/.com/.bin). Gzipped and\n");
printf("                        zipped (.gz and .zip) are also supported.\n");
printf(" -frameskip 1 - 9     : Options to skip some frames. Default is 1.\n");
#ifndef DINGUX
printf(" -bpp 0,8,15,16,24,32 : Set the bpp backend. 0 = autodetect\n");
printf(" -rtype 1,2,3         : Select rendering. 1 = SDL, 2 = OpenGL, 3 = YUV\n");
printf(" -[no]fullscreen      : Disable/Enable fullscreen rendering mode.\n");
#endif
printf(" -[no]throttle        : Disable/Enable throttle to a max of 60FPS.\n");
printf(" -[no]sound           : Disable/Enable sound support.\n");
#ifndef DINGUX
printf(" -[no]autoskip        : Disable/Enable auto-frameskip\n");
printf("\n");
printf(" SDL Rendering options:\n");
printf("\n");
printf(" -stype 1,2,3         : Select Scaling/Scanline routine\n");
printf("                        1 = SDLEMU v1 (compatible with all SDL versions)\n");
printf("                        2 = SDLEMU v2 (possibly faster but can be broken)\n");
printf("                        3 = Pierre Doucet v1 (compatible, but possibly slow)\n");
printf(" -[no]fps             : Disable/Enable FPS counter (windowed only).\n");
printf("                        Warning : cpu hungry.\n");
printf(" -2 (or -3 or -4)     : Scale display by 2 (or 3 or 4)\n");
printf(" -[no]lcd             : Disable/Enable LCD display emulation\n");
printf("\n");
printf(" Additional Rendering Filters (only for SDL Rendering):\n");
printf(" WARNING! These filters are forced to work with -bpp 16 and with (scale) -2\n");
printf("\n");
printf(" -filter 1 - 10       : Enable the SDL enhancement/effect filter\n");
printf("                         1 = TV Mode,    2 = 2xSAI,        3 = Super2xSAI, \n");
printf("                         4 = SuperEagle, 5 = MotionBlur,   6 = Simple2x, \n");
printf("                         7 = Bilinear,   8 = BilinearPlus, 9 = Pixelate, \n");
printf("                        10 = Average\n");
printf("\n");
printf(" OpenGL Rendering options:\n");
printf("\n");
printf(" -[no]fsaa            : Disable/Enable FSAA filter in OpenGL mode\n");
printf(" -[no]accel           : Disable/Enable Hardware Acceleration in OpenGL mode\n");
printf("                        With this you can force hardware acceleration! Be warned,\n");
printf("                        this can break performance and can result in crashes!\n");
printf(" -[no]sync            : Disable/Enable VSYNC in OpenGL mode\n");
printf("                        With this you can force OpenGL to sync against the monitor\n");
printf("                        refreshrate, so that it never breaks,above your monitor specs.\n");
printf("                        This can fix tearing during rendering in OpenGL\n");
printf(" YUV Overlay Rendering options:\n");
printf("\n");
printf(" -format 1 - 5        : Select the desired YUV Overlay format\n");
printf("                        1 = YV12, 2 = IYUV, 3 = YUY2\n");
printf("                        4 = UYVY, 5 = YVYU\n");
#endif

}
