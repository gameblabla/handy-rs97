/*

	Main menu items:
	LOAD ROM
	CONFIG
	LOAD STATE: 0
	SAVE STATE: 0
	RESET
	EXIT

	Config menu items;
	IMAGE SCALING: x2 / FULLSCREEN
	FRAMESKIP: 1 - 9
	SHOW FPS: YES / NO
	LIMIT FPS: YES / NO
	SWAP A/B: YES / NO


*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include <errno.h>

#include "gui.h"
#include "font.h"

#include "../handy_sdl_main.h"
#include "../handy_sdl_graphics.h"

/* defines and macros */
#define MAX__PATH 1024
#define FILE_LIST_ROWS 24
#define FILE_LIST_POSITION 8
#define DIR_LIST_POSITION 208

#define color16(red, green, blue) ((red << 11) | (green << 5) | blue)

#define COLOR_BG            color16(05, 03, 02)
#define COLOR_ROM_INFO      color16(22, 36, 26)
#define COLOR_ACTIVE_ITEM   color16(31, 63, 31)
#define COLOR_INACTIVE_ITEM color16(13, 40, 18)
#define COLOR_FRAMESKIP_BAR color16(15, 31, 31)
#define COLOR_HELP_TEXT     color16(16, 40, 24)

/* external references */
extern int Throttle; // show fps, from handy_sdl_main.cpp
extern char rom_name_with_no_ext[128]; // name if current rom, used for load/save state

/* SDL declarations */
extern SDL_Surface *HandyBuffer; // Our Handy/SDL display buffer
extern SDL_Surface *mainSurface; // Our Handy/SDL primary display
SDL_Surface *menuSurface = NULL, *Game_Surface_Preview; // menu rendering

int Invert = 0;

void gui_LoadState();
void gui_SaveState();
void gui_FileBrowserRun();
void gui_ConfigMenuRun();
void gui_Reset();
void gui_Init();
void gui_video_early_init();
void gui_Flip();
void print_string(const char *s, u16 fg_color, u16 bg_color, int x, int y);
void get_config_path();

int gui_LoadSlot = 0;
int gui_ImageScaling = 0;
int gui_Frameskip = 0;
int gui_FPS = 0;
int gui_Show_FPS = 0;
int gui_LimitFPS = 0;
int gui_SwapAB = 0;

int loadslot = -1; // flag to reload preview screen
int done = 0; // flag to indicate exit status

char config_full_path[MAX__PATH];

typedef struct {
	const char *itemName;
	int *itemPar;
	int itemParMaxValue;
	const char **itemParName;
	void (*itemOnA)();
} MENUITEM;

typedef struct {
	int itemNum; // number of items
	int itemCur; // current item
	MENUITEM *m; // array of items
} MENU;

#ifdef RS90
const char* gui_ScaleNames[] = {"Original", "Fullscreen"};
#else
const char* gui_ScaleNames[] = {"Prefer Aspect", "Fullscreen"};
#endif
const char* gui_YesNo[] = {"no", "yes"};

MENUITEM gui_MainMenuItems[] = {
	/* It's unusable on the RS-90, disable */
	#ifndef NOROMLOADER
	{(const char *)"Load rom", NULL, NULL, NULL, &gui_FileBrowserRun},
	#endif
	{(const char *)"Config", NULL, NULL, NULL, &gui_ConfigMenuRun},
	{(const char *)"Load state: ", &gui_LoadSlot, 9, NULL, &gui_LoadState},
	{(const char *)"Save state: ", &gui_LoadSlot, 9, NULL, &gui_SaveState},
	{(const char *)"Reset", NULL, NULL, NULL, &gui_Reset},
	{(const char *)"Exit", NULL, NULL, NULL, &handy_sdl_quit} // extern in handy_sdl_main.cpp
};

#ifdef NOROMLOADER
MENU gui_MainMenu = { 5, 0, (MENUITEM *)&gui_MainMenuItems };
#else
MENU gui_MainMenu = { 6, 0, (MENUITEM *)&gui_MainMenuItems };
#endif

MENUITEM gui_ConfigMenuItems[] = {
	{(const char *)"Upscale  : ", &gui_ImageScaling, 1, (const char **)&gui_ScaleNames, NULL},
	{(const char *)"Swap A/B : ", &gui_SwapAB, 1, (const char **)&gui_YesNo, NULL}
};

MENU gui_ConfigMenu = { 2, 0, (MENUITEM *)&gui_ConfigMenuItems };

/*
	Clears mainSurface
*/
void gui_ClearScreen()
{
	uint8_t i;
	for(i=0;i<4;i++)
	{
		SDL_FillRect(mainSurface,NULL,SDL_MapRGBA(mainSurface->format, 0, 0, 0, 255));
		SDL_Flip(mainSurface);
	}
}

/*
	Prints char on a given surface
*/
void ShowChar(SDL_Surface *s, int x, int y, unsigned char a, int fg_color, int bg_color)
{
	Uint16 *dst;
	int w, h;

	if(SDL_MUSTLOCK(s)) SDL_LockSurface(s);
	for(h = 8; h; h--) {
		dst = (Uint16 *)s->pixels + (y+8-h)*s->w + x;
		for(w = 8; w; w--) {
			Uint16 color = bg_color; // background
			if((gui_font[a*8 + (8-h)] >> w) & 1) color = fg_color; // test bits 876543210
			*dst++ = color;
		}
	}
	if(SDL_MUSTLOCK(s)) SDL_UnlockSurface(s);
}

void ShowString(int x, int y, const char *s)
{
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 8) ShowChar(menuSurface, x, y, s[i], 0xFFFF, 0);
}

void ShowStringEx(int x, int y, const char *s)
{
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 8) ShowChar(mainSurface, x, y, s[i], 0xFFFF, 0);
}

void ShowMenuItem(int x, int y, MENUITEM *m, int fg_color)
{
	static char i_str[24];

	// if no parameters, show simple menu item
	if(m->itemPar == NULL) print_string(m->itemName, fg_color, COLOR_BG, x, y);
	else {
		if(m->itemParName == NULL) {
			// if parameter is a digit
			sprintf(i_str, "%s%i", m->itemName, *m->itemPar);
		} else {
			// if parameter is a name in array
			sprintf(i_str, "%s%s", m->itemName, *(m->itemParName + *m->itemPar));
		}
		print_string(i_str, fg_color, COLOR_BG, x, y);
	}
}

void gui_LoadState()
{
	char savename[512];

	sprintf(savename, "%s/%s.%i.sav", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
	//printf("Load state: %s\n", savename);

	// check if file exists otherwise mpLynx->ContextLoad will crash
	FILE *fp = fopen(savename, "rb");
	if(!fp) return;
	fclose(fp);

	if(!mpLynx->ContextLoad(savename)) printf("Error loading state: %s\n", savename);
		else { mpLynx->SetButtonData(0); done = TRUE; }
}

void gui_SaveState()
{
	char savename[512];

	sprintf(savename, "%s/%s.%i.bmp", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
				
	SDL_SaveBMP(HandyBuffer, savename);

	sprintf(savename, "%s/%s.%i.sav", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
	//printf("Save state: %s\n", savename);

	if(!mpLynx->ContextSave(savename)) printf("Error saving state: %s\n", savename); //else done = TRUE;

	loadslot = -1; // show preview immediately
}

/*
	copy-pasted mostly from gpsp emulator by Exophaze
	thanks for it
*/
void print_string(const char *s, u16 fg_color, u16 bg_color, int x, int y)
{
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 8) ShowChar(menuSurface, x, y, s[i], fg_color, bg_color);
}

int sort_function(const void *dest_str_ptr, const void *src_str_ptr)
{
	char *dest_str = *((char **)dest_str_ptr);
	char *src_str = *((char **)src_str_ptr);

	if(src_str[0] == '.') return 1;

	if(dest_str[0] == '.') return -1;

	return strcasecmp(dest_str, src_str);
}

s32 load_file(const char **wildcards, char *result)
{
	SDL_Event gui_event;
	char current_dir_name[MAX__PATH];
	DIR *current_dir;
	struct dirent *current_file;
	struct stat file_info;
	char current_dir_short[81];
	u32 current_dir_length;
	u32 total_filenames_allocated;
	u32 total_dirnames_allocated;
	char **file_list;
	char **dir_list;
	u32 num_files;
	u32 num_dirs;
	char *file_name;
	u32 file_name_length;
	u32 ext_pos = -1;
	u32 chosen_file, chosen_dir;
	u32 dialog_result = 1;
	s32 return_value = 1;
	u32 current_file_selection;
	u32 current_file_scroll_value;
	u32 current_dir_selection;
	u32 current_dir_scroll_value;
	u32 current_file_in_scroll;
	u32 current_dir_in_scroll;
	u32 current_file_number, current_dir_number;
	u32 current_column = 0;
	u32 repeat;
	u32 i;

	while(return_value == 1) {
		current_file_selection = 0;
		current_file_scroll_value = 0;
		current_dir_selection = 0;
		current_dir_scroll_value = 0;
		current_file_in_scroll = 0;
		current_dir_in_scroll = 0;

		total_filenames_allocated = 32;
		total_dirnames_allocated = 32;
		file_list = (char **)malloc(sizeof(char *) * 32);
		dir_list = (char **)malloc(sizeof(char *) * 32);
		memset(file_list, 0, sizeof(char *) * 32);
		memset(dir_list, 0, sizeof(char *) * 32);

		num_files = 0;
		num_dirs = 0;
		chosen_file = 0;
		chosen_dir = 0;

		getcwd(current_dir_name, MAX__PATH);
		current_dir = opendir(current_dir_name);

		// DEBUG
		//printf("Current directory: %s\n", current_dir_name);

		do 
		{
			if (current_dir)
			{
				current_file = readdir(current_dir);
			}
			else
			{
				current_file = NULL;
			}

			if(current_file) {
				file_name = current_file->d_name;
				file_name_length = strlen(file_name);

				if((stat(file_name, &file_info) >= 0) && ((file_name[0] != '.') || (file_name[1] == '.'))) {
					if(S_ISDIR(file_info.st_mode)) {
						dir_list[num_dirs] = (char *)malloc(file_name_length + 1);
						strcpy(dir_list[num_dirs], file_name);

						num_dirs++;
					} else {
					// Must match one of the wildcards, also ignore the .
						if(file_name_length >= 4) {
							if(file_name[file_name_length - 4] == '.') ext_pos = file_name_length - 4;
							else if(file_name[file_name_length - 3] == '.') ext_pos = file_name_length - 3;
							else ext_pos = 0;

							for(i = 0; wildcards[i] != NULL; i++) {
								if(!strcasecmp((file_name + ext_pos), wildcards[i])) {
									file_list[num_files] = (char *)malloc(file_name_length + 1);

									strcpy(file_list[num_files], file_name);

									num_files++;
									break;
								}
							}
						}
					}
				}

				if(num_files == total_filenames_allocated) {
					file_list = (char **)realloc(file_list, sizeof(char *) * total_filenames_allocated * 2);
					memset(file_list + total_filenames_allocated, 0, sizeof(char *) * total_filenames_allocated);
					total_filenames_allocated *= 2;
				}

				if(num_dirs == total_dirnames_allocated) {
					dir_list = (char **)realloc(dir_list, sizeof(char *) * total_dirnames_allocated * 2);
					memset(dir_list + total_dirnames_allocated, 0, sizeof(char *) * total_dirnames_allocated);
					total_dirnames_allocated *= 2;
				}
			}
		} while(current_file);

		qsort((void *)file_list, num_files, sizeof(char *), sort_function);
		qsort((void *)dir_list, num_dirs, sizeof(char *), sort_function);

		// DEBUG
		//for(i = 0; i < num_dirs; i++) printf("%s\n", dir_list[i]);
		//for(i = 0; i < num_files; i++) printf("%s\n", file_list[i]);

		if (current_dir) closedir(current_dir);

		current_dir_length = strlen(current_dir_name);

		if(current_dir_length > 80) {
			memcpy(current_dir_short, "...", 3);
			memcpy(current_dir_short + 3, current_dir_name + current_dir_length - 77, 77);
			current_dir_short[80] = 0;
		} else {
			memcpy(current_dir_short, current_dir_name, current_dir_length + 1);
		}

		repeat = 1;

		if(num_files == 0) current_column = 1;

		//clear_screen(COLOR_BG);
		char print_buffer[81];

		while(repeat) {
			//flip_screen();
			SDL_FillRect(menuSurface, NULL, COLOR_BG);
			print_string(current_dir_short, COLOR_ACTIVE_ITEM, COLOR_BG, 0, 0);
			print_string("Press SELECT/B to return to the main menu", COLOR_HELP_TEXT, COLOR_BG, 5, 220);
			for(i = 0, current_file_number = i + current_file_scroll_value; i < FILE_LIST_ROWS; i++, current_file_number++) {
				if(current_file_number < num_files) {
					strncpy(print_buffer,file_list[current_file_number], 38);
					print_buffer[38] = 0;
					if((current_file_number == current_file_selection) && (current_column == 0)) {
						print_string(print_buffer, COLOR_ACTIVE_ITEM, COLOR_BG, FILE_LIST_POSITION, ((i + 2) * 8));
					} else {
						print_string(print_buffer, COLOR_INACTIVE_ITEM, COLOR_BG, FILE_LIST_POSITION, ((i + 2) * 8));
					}
				}
			}
			for(i = 0, current_dir_number = i + current_dir_scroll_value; i < FILE_LIST_ROWS; i++, current_dir_number++) {
				if(current_dir_number < num_dirs) {
					strncpy(print_buffer,dir_list[current_dir_number], 13);
					print_buffer[14] = 0;
					if((current_dir_number == current_dir_selection) && (current_column == 1)) {
						print_string(print_buffer, COLOR_ACTIVE_ITEM, COLOR_BG, DIR_LIST_POSITION, ((i + 2) * 8));
					} else {
						print_string(print_buffer, COLOR_INACTIVE_ITEM, COLOR_BG, DIR_LIST_POSITION, ((i + 2) * 8));
					}
				}
			}

			// Catch input
			// change to read key state later
			while(SDL_PollEvent(&gui_event)) {
				if(gui_event.type == SDL_KEYDOWN) 
				{
					// DINGOO A - apply parameter or enter submenu
					if(gui_event.key.keysym.sym == SDLK_RETURN || gui_event.key.keysym.sym == SDLK_LCTRL) 
					{
						if(current_column == 1) {
							repeat = 0;
							chdir(dir_list[current_dir_selection]);
						} else {
							if(num_files != 0) {
								repeat = 0;
								return_value = 0;
								//strcpy(result, file_list[current_file_selection]);
								sprintf(result, "%s/%s", current_dir_name, file_list[current_file_selection]);
								break;
							}
						}
					}
					// DINGOO B - exit or back to previous menu
					if(gui_event.key.keysym.sym == SDLK_ESCAPE || gui_event.key.keysym.sym == SDLK_LALT) 
					{
						return_value = -1;
						repeat = 0;
						break;
					}
					if(gui_event.key.keysym.sym == SDLK_UP) { // DINGOO UP - arrow down
						if(current_column == 0) {
							if(current_file_selection) {
								current_file_selection--;
								if(current_file_in_scroll == 0) {
									//clear_screen(COLOR_BG);
									current_file_scroll_value--;
								} else {
									current_file_in_scroll--;
								}
							}
						} else {
							if(current_dir_selection) {
								current_dir_selection--;
								if(current_dir_in_scroll == 0) {
									//clear_screen(COLOR_BG);
									current_dir_scroll_value--;
								} else {
									current_dir_in_scroll--;
								}
							}
						}
					}
					if(gui_event.key.keysym.sym == SDLK_DOWN) { // DINGOO DOWN - arrow up
						if(current_column == 0) {
							if(current_file_selection < (num_files - 1)) {
								current_file_selection++;
								if(current_file_in_scroll == (FILE_LIST_ROWS - 1)) {
									//clear_screen(COLOR_BG);
									current_file_scroll_value++;
								} else {
									current_file_in_scroll++;
								}
							}
						} else {
							if(current_dir_selection < (num_dirs - 1)) {
								current_dir_selection++;
								if(current_dir_in_scroll == (FILE_LIST_ROWS - 1)) {
									//clear_screen(COLOR_BG);
									current_dir_scroll_value++;
								} else {
									current_dir_in_scroll++;
								}
							}
						}
					}
					if(gui_event.key.keysym.sym == SDLK_LEFT) { // DINGOO LEFT - decrease parameter value
						if(current_column == 1) {
							if(num_files != 0) current_column = 0;
						}
					}
					if(gui_event.key.keysym.sym == SDLK_RIGHT) { // DINGOO RIGHT - increase parameter value
						if(current_column == 0) {
							if(num_dirs != 0) current_column = 1;
						}
					}
				}
			}

			SDL_Delay(16);
			gui_Flip();
		}

		// free pointers
		for(i = 0; i < num_files; i++)
		{
			if (file_list[i])
			{
				free(file_list[i]);
				file_list[i] = NULL;
			}
		}
		
		if (file_list)
		{
			free(file_list);
			file_list = NULL;
		}

		for(i = 0; i < num_dirs; i++)
		{
			if (dir_list[i])
			{
				free(dir_list[i]);
				dir_list[i] = NULL;
			}
		}
		
		if (dir_list)
		{
			free(dir_list);
			dir_list = NULL;
		}
	}

	return return_value;
}

/*
	Rom file browser which is called from menu
*/
const char* file_ext[4] = { ".lnx", ".lyx", ".zip", NULL };

void gui_FileBrowserRun()
{

	static char load_filename[512];

	if(load_file(file_ext, load_filename) != -1) // exit if file is chosen
	{ 
		handy_sdl_core_reinit(load_filename);
		gui_ClearScreen();
		done = TRUE;
		loadslot = -1;
	}
	//printf("File chosen: %s\n", load_filename);
}

/*
	Rom browser which is called FIRST before all other init
	Return values :		1 - file chosen, name is written at *romname
						0 - no file
*/
int gui_LoadFile(char *romname)
{
	int result = 0;

	// get current working dir before it's modified by load_file
	get_config_path();

	if(load_file(file_ext, romname) != -1) result = 1;

	return result;
}

/*
	Shows previews of load/save and pause
*/
void ShowPreview(MENU *menu)
{
	char prename[256];
	static uint8_t prebuffer[160 * 160 * 2];
	uint32_t i;
	SDL_Rect dst, dst2;
	SDL_Surface *tmp;

	if(menu == &gui_MainMenu && (menu->itemCur == 2 || menu->itemCur == 3)) 
	{
		if(loadslot != gui_LoadSlot) 
		{
			// create preview name
			sprintf(prename, "%s/%s.%i.bmp", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
			
			if (Game_Surface_Preview)
			{
				SDL_FreeSurface(Game_Surface_Preview);
				Game_Surface_Preview = NULL;
			}
			
			// check if file exists
			tmp = SDL_LoadBMP(prename);
			if (tmp)
			{
				Game_Surface_Preview = SDL_DisplayFormat(tmp);
				SDL_FreeSurface(tmp);
				tmp = NULL;
			}
			loadslot = gui_LoadSlot; // do not load img file each time
		}
		
		if (Game_Surface_Preview) 
		{
			#ifdef RS90
			dst.x = 72;
			dst.y = 16;
			dst.w = 96;
			dst.h = 64;
			dst2.x = 0;
			dst2.y = 0;
			dst2.w = LynxWidth;
			dst2.h = LynxHeight;
			#else
			dst.x = 80;
			dst.y = 24;
			dst.w = 160;
			dst.h = 102;
			dst2.x = 0;
			dst2.y = 0;
			dst2.w = LynxWidth;
			dst2.h = LynxHeight;
			#endif
			SDL_SoftStretch(Game_Surface_Preview, &dst2, menuSurface, &dst);
		}
		//for(int y = 0; y < 102; y++) memcpy((char *)menuSurface->pixels + (24 + y) * 320*2 + 80*2, prebuffer + y * 320, 320);
	}
	else
	{
		if (HandyBuffer) 
		{
			#ifdef RS90
			dst.x = 72;
			dst.y = 16;
			dst.w = 96;
			dst.h = 64;
			dst2.x = 0;
			dst2.y = 0;
			dst2.w = LynxWidth;
			dst2.h = LynxHeight;
			#else
			dst.x = 80;
			dst.y = 24;
			dst.w = 160;
			dst.h = 102;
			dst2.x = 0;
			dst2.y = 0;
			dst2.w = LynxWidth;
			dst2.h = LynxHeight;
			#endif
			SDL_SoftStretch(HandyBuffer, &dst2, menuSurface, &dst);
		}
	}
}

/*
	Shows menu items and pointing arrow
*/
void ShowMenu(MENU *menu)
{
	int i;
	MENUITEM *mi = menu->m;

	// clear buffer
	SDL_FillRect(menuSurface, NULL, COLOR_BG);

	// show menu lines
	for(i = 0; i < menu->itemNum; i++, mi++) {
		int fg_color;

		if(menu->itemCur == i) fg_color = COLOR_ACTIVE_ITEM; else fg_color = COLOR_INACTIVE_ITEM;
	#ifdef RS90
		ShowMenuItem(56, (13 + i) * 8, mi, fg_color);
	#else
		ShowMenuItem(80, (18 + i) * 8, mi, fg_color);
	#endif
	}

	// show preview screen
	ShowPreview(menu);

	// print info string
	#ifdef RS90
	print_string("Handy : " __DATE__ " build", COLOR_HELP_TEXT, COLOR_BG, 20, 2);
	print_string("Port by gameblabla", COLOR_HELP_TEXT, COLOR_BG, 48, 88);
	print_string("[B] = Return to game", COLOR_HELP_TEXT, COLOR_BG, 4, 158-8);
	#else
	print_string("Press B to return to the game", COLOR_HELP_TEXT, COLOR_BG, 56, 220);
	print_string("Handy RS-97 libretro", COLOR_HELP_TEXT, COLOR_BG, 80, 2);
	print_string("Port by gameblabla", COLOR_HELP_TEXT, COLOR_BG, 80, 12);
	#endif
}

/*
	Main function that runs all the stuff
*/
void gui_MainMenuRun(MENU *menu)
{
	SDL_Event gui_event;
	MENUITEM *mi;

	done = FALSE;

	while(!done) 
	{
		mi = menu->m + menu->itemCur; // pointer to highlit menu option

		while(SDL_PollEvent(&gui_event)) 
		{
			if(gui_event.type == SDL_KEYDOWN) {
				// DINGOO A - apply parameter or enter submenu
				//if(gui_event.key.keysym.sym == SDLK_RETURN) if(mi->itemOnA != NULL) (*mi->itemOnA)();
				if(gui_event.key.keysym.sym == SDLK_LCTRL)
				{
					if(mi->itemOnA != NULL) (*mi->itemOnA)();
				}
				// DINGOO B - exit or back to previous menu
				//if(gui_event.key.keysym.sym == SDLK_ESCAPE) return;
				if(gui_event.key.keysym.sym == SDLK_LALT) return;
				// DINGOO UP - arrow down
				if(gui_event.key.keysym.sym == SDLK_UP) if(--menu->itemCur < 0) menu->itemCur = menu->itemNum - 1;
				// DINGOO DOWN - arrow up
				if(gui_event.key.keysym.sym == SDLK_DOWN) if(++menu->itemCur == menu->itemNum) menu->itemCur = 0;
				// DINGOO LEFT - decrease parameter value
				if(gui_event.key.keysym.sym == SDLK_LEFT) {
					if(mi->itemPar != NULL && *mi->itemPar > 0) *mi->itemPar -= 1;
				}
				// DINGOO RIGHT - increase parameter value
				if(gui_event.key.keysym.sym == SDLK_RIGHT) {
					if(mi->itemPar != NULL && *mi->itemPar < mi->itemParMaxValue) *mi->itemPar += 1;
				}
			}
		}
		if(!done) ShowMenu(menu); // show menu items
		SDL_Delay(16);
		gui_Flip();
	}
	SDL_FillRect(mainSurface, NULL, 0);
	SDL_Flip(mainSurface);
	SDL_FillRect(mainSurface, NULL, 0);
	SDL_Flip(mainSurface);
	#ifdef SDL_TRIPLEBUF
	SDL_FillRect(mainSurface, NULL, 0);
	SDL_Flip(mainSurface);
	#endif
}

void get_config_path()
{
	// 1) get HOME environment, check if it's read-only
	// if yes, 2) check if current working directory is read-only
	// if yes 3) change to /usr/etc

	// current working dir may be already set (lame check)
	if(strlen(config_full_path) == 0) {

		// check HOME
		#ifndef WIN32
		char *env = getenv("HOME");

		// if HOME found, append to config_full_path
		if(env != NULL) strcat(config_full_path, env);
		strcat(config_full_path, "/.handy"); 
		mkdir(config_full_path, 0755);

		// return if not read-only, otherwise we are on rzx50 or a380 dingux
		if(errno != EROFS && errno != EACCES && errno != EPERM) return;
		memset(config_full_path, 0 , 512);
		#endif

		// check current working dir
		getcwd(config_full_path, MAX__PATH);
		strcat(config_full_path, "/.handy");
		mkdir(config_full_path
		#ifndef WIN32
		, 0755
		#endif
		);

	}

	// DEBUG
	//printf("Config and save dir: %s\n", config_full_path);
}

void gui_Init()
{
	get_config_path();
}

void gui_Run()
{
	extern int filter; // remove later, temporal hack
	extern int BT_A, BT_B; // remove later, temporal hack

	SDL_EnableKeyRepeat(/*SDL_DEFAULT_REPEAT_DELAY*/ 150, /*SDL_DEFAULT_REPEAT_INTERVAL*/30);
	gui_ClearScreen();
	gui_MainMenuRun(&gui_MainMenu);
	if (gui_SwapAB == 0) 
	{
		BT_A = SDLK_LCTRL;
		BT_B = SDLK_LALT;
		//BT_A = SDLK_p;
		//BT_B = SDLK_o;
	}
	else 
	{
		BT_A = SDLK_LALT;
		BT_B = SDLK_LCTRL;
		//BT_A = SDLK_o;
		//BT_B = SDLK_p;
	}
	gui_ClearScreen();
	SDL_EnableKeyRepeat(0, 0);
}

void gui_ConfigMenuRun()
{
	gui_MainMenuRun(&gui_ConfigMenu);
}

void gui_Reset()
{
	mpLynx->Reset();
	done = TRUE; // mark to exit
}

void gui_Flip()
{
	SDL_Rect dstrect;
	//dstrect.x = (mainSurface->w - 320) / 2;
	//dstrect.y = (mainSurface->h - 240) / 2;
	if (mainSurface->w == 320 || mainSurface->w == 240)
	{
		SDL_BlitSurface(menuSurface, NULL, mainSurface, NULL);
	}
	else
	{
		SDL_SoftStretch(menuSurface, NULL, mainSurface, NULL);
	}
	SDL_Flip(mainSurface);
}
