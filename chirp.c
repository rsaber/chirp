/*
	chirp - a chip8 emulator

	Uses SDL for graphics.

	Written by Riyasat Saber
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "chip8.h"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32

// Globals
unsigned int global_foreground_colour = 0xFFFFFF;
unsigned int global_background_colour = 0x000000;
unsigned int global_scale = 10;

Chip c;
uc km[16];

uc screen[WINDOW_HEIGHT][WINDOW_WIDTH];

// Used for redrawing after reshape
unsigned int display_width = WINDOW_WIDTH;
unsigned int display_height = WINDOW_HEIGHT;

// Func Prototypes
void loadKeyMappings(char *);
void click_key(SDL_KeyboardEvent *key );
void read_settings(int, char **);
void drawMonochromeScaledPixel(SDL_Renderer * r, char col, int x, int y, int scale);
void display(SDL_Renderer * r);

int main(int argc, char ** argv){
	printf("chirp - a CHIP-8 emulator written in C\nCopyright (C) 2016 Riyasat Saber\n\nEsc to exit\n");
	
	if(argc < 2){
		printf("No Game Cartridge found!\n"
				"Run with: %s [filepath]\n", argv[0]);
		return -1;
	}

	if(argc>=2)
		read_settings(argc, argv);

	c = newChip();
	loadProgram(c, argv[1]);

	loadKeyMappings("key_mappings");

	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;
	int i;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(WINDOW_WIDTH*global_scale, WINDOW_HEIGHT*global_scale, 0, &window, &renderer);
	
	//SDL_RenderSetScale(renderer,global_scale,global_scale);
	// Really ugly, will find a neater solution later
	char *title = calloc(sizeof(char) , (strlen(argv[1]) + 9));
	strcat(title, "chirp - ");
	strcat(title, argv[1]);

	SDL_SetWindowTitle(window, title);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	free(title);

	uc quit = 0;
	while(!quit){
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					click_key(&event.key);
					break;

				case SDL_QUIT:
					quit = !quit;
					break;

				default:
					break;
			}
		}
		cycle(c);
		display(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

	return 0; 
}

void display(SDL_Renderer * r){
	// if draw flag isnt set, no need to draw this cycle
	if(!c->draw_flag) return;

	int i,j;
	for(int i=0; i<WINDOW_HEIGHT; i++){
		for(int j=0; j<WINDOW_WIDTH; j++){
			// Draw a coloured pixel
			drawMonochromeScaledPixel(r,c->display[(i*64)+j],j,i,global_scale);
			
			/*if(c->display[(i*64)+j]) SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
			else SDL_SetRenderDrawColor(r, 0, 0, 0, 0);

			SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
			SDL_RenderDrawPoint(r,i,j);*/
		}
	}
}


// char col is either 0 or 1
void drawMonochromeScaledPixel(SDL_Renderer * r, char col, int x, int y, int scale){
	if(!col) SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
	else SDL_SetRenderDrawColor(r, 255, 255, 255, 255);

	int i,j;
	x*=scale;
	y*=scale;
	for(i=0; i<scale; i++){
		for(j=0; j<scale; j++){
			SDL_RenderDrawPoint(r,x+i,y+j);
		}
	}
}

void loadKeyMappings(char * filename){
	FILE * fp = fopen(filename,"r");
	if(fp==NULL){
		fprintf(stderr, "Key Mappings not found!\n");
		exit(0);
	}

	uc i = 0;
	while(i<16) km[i++] = fgetc(fp);

	fclose(fp);
}


void read_settings(int argc, char ** strings){
	int i;
	for(i=3; i<argc; i++){
		if(strstr(strings[i], "-FCOLOUR=") == strings[i]){
			global_foreground_colour = atoi(strings[i] + 9);
		}else if(strstr(strings[i], "-BCOLOUR=") == strings[i]){
			global_background_colour = atoi(strings[i] + 9);
		}else if(strstr(strings[i], "-SCALE=") == strings[i]){
			global_scale = atoi(strings[i] +  7);
		}else if(strstr(strings[i], "help") == strings[i]){
			printf("List of flags:\n"
					" -FCOLOUR=0xABCDEF : set the foreground colour to 0xABCDEF\n"
					" -BCOLOUR=0xABCDEF : set the background colour to 0xABCDEF\n"
					" -SCALE=ABC       : set the scaling to ABC (nearest neighbour)"
					" help             : this help message");
		}else{
			printf("Invalid flag : %s\n",strings[i]);
		}
	}
	printf("Setting foreground colour to %x\n", global_foreground_colour);
	printf("Setting background colour to %x\n", global_background_colour);
	printf("Setting global scale to %d\n", global_scale);
}

void click_key(SDL_KeyboardEvent * key){
	// Check whether a key was pressed or released
	uc v = key->type == SDL_KEYUP ? 0 : 1;
	printf("Key Clicked! %d\n",v);
	switch(key->keysym.sym){
		case SDLK_1:
			c->key[0x1] = v;
			break;
		case SDLK_2:
			c->key[0x2] = v;
			break;
		case SDLK_3:
			c->key[0x3] = v;
			break;
		case SDLK_4:
			c->key[0xC] = v;
			break;

		case SDLK_q:
			c->key[0x4] = v;
			break;
		case SDLK_w:
			c->key[0x5] = v;
			break;
		case SDLK_e:
			c->key[0x6] = v;
			break;
		case SDLK_r:
			c->key[0xD] = v;
			break;

		case SDLK_a:
			c->key[0x7] = v;
			break;
		case SDLK_s:
			c->key[0x8] = v;
			break;
		case SDLK_d:
			c->key[0x9] = v;
			break;
		case SDLK_f:
			c->key[0xE] = v;
			break;

		case SDLK_z:
			c->key[0xA] = v;
			break;
		case SDLK_x:
			c->key[0x0] = v;
			break;
		case SDLK_c:
			c->key[0xB] = v;
			break;
		case SDLK_v:
			c->key[0xF] = v;
			break;

		case SDLK_ESCAPE:
			exit(0);
			break;
		
	}
}

/* Old Version, tried OpenGL and GLUT */
/*
void key_down(uc key, int x, int y){
	if(key == 27) 			exit(0);
	else if(key == km[0])	c->key[0x1] = 1;
	else if(key == km[1])	c->key[0x2] = 1;
	else if(key == km[2])	c->key[0x3] = 1;
	else if(key == km[3])	c->key[0xC] = 1;

	else if(key == km[4])	c->key[0x4] = 1;
	else if(key == km[5])	c->key[0x5] = 1;
	else if(key == km[6])	c->key[0x6] = 1;
	else if(key == km[7])	c->key[0xD] = 1;
	else if(key == km[8])	c->key[0x7] = 1;
	else if(key == km[9])	c->key[0x8] = 1;
	else if(key == km[10])	c->key[0x9] = 1;
	else if(key == km[11])	c->key[0xE] = 1;
	else if(key == km[12])	c->key[0xA] = 1;
	else if(key == km[13])	c->key[0x0] = 1;
	else if(key == km[14])	c->key[0xB] = 1;
	else if(key == km[15])	c->key[0xF] = 1;
}

void key_up(uc key, int x, int y){
	if(key == 27)			exit(0);
	else if(key == km[0])	c->key[0x1] = 0;
	else if(key == km[1])	c->key[0x2] = 0;
	else if(key == km[2])	c->key[0x3] = 0;
	else if(key == km[3])	c->key[0xC] = 0;
	else if(key == km[4])	c->key[0x4] = 0;
	else if(key == km[5])	c->key[0x5] = 0;
	else if(key == km[6])	c->key[0x6] = 0;
	else if(key == km[7])	c->key[0xD] = 0;
	else if(key == km[8])	c->key[0x7] = 0;
	else if(key == km[9])	c->key[0x8] = 0;
	else if(key == km[10])	c->key[0x9] = 0;
	else if(key == km[11])	c->key[0xE] = 0;
	else if(key == km[12])	c->key[0xA] = 0;
	else if(key == km[13])	c->key[0x0] = 0;
	else if(key == km[14])	c->key[0xB] = 0;
	else if(key == km[15])	c->key[0xF] = 0;
}

*/
