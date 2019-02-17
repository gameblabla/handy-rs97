/*
	Header for gui.cpp
*/
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef uint32_t u32;
typedef signed int s32;
typedef unsigned long long int u64;
typedef signed long long int s64;

void gui_CountFPS();
void gui_ShowFPS();

void gui_Run();
void gui_Init(); 

int gui_LoadFile(char *romname);
