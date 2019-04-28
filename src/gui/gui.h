/*
	Header for gui.cpp
*/
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef unsigned long long int u64;
typedef signed long long int s64;

void gui_CountFPS();
void gui_ShowFPS();

void gui_Run();
void gui_Init(); 

int gui_LoadFile(char *romname);
