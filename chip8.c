/*
	CHIP-8 Machine Emulator
	Written by Riyasat Saber
*/

#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"

// Need to load fontset into first 512 bytes of memory
unsigned char fontset[80] ={ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, 
  0x20, 0x60, 0x20, 0x20, 0x70, 
  0xF0, 0x10, 0xF0, 0x80, 0xF0, 
  0xF0, 0x10, 0xF0, 0x10, 0xF0, 
  0x90, 0x90, 0xF0, 0x10, 0x10, 
  0xF0, 0x80, 0xF0, 0x10, 0xF0, 
  0xF0, 0x80, 0xF0, 0x90, 0xF0, 
  0xF0, 0x10, 0x20, 0x40, 0x40, 
  0xF0, 0x90, 0xF0, 0x90, 0xF0, 
  0xF0, 0x90, 0xF0, 0x10, 0xF0, 
  0xF0, 0x90, 0xF0, 0x90, 0x90, 
  0xE0, 0x90, 0xE0, 0x90, 0xE0, 
  0xF0, 0x80, 0x80, 0x80, 0xF0, 
  0xE0, 0x90, 0x90, 0x90, 0xE0, 
  0xF0, 0x80, 0xF0, 0x80, 0xF0, 
  0xF0, 0x80, 0xF0, 0x80, 0x80  
};

Chip newChip(){
	Chip c = malloc(sizeof(struct _chip));
	
	c->pc = MAP_PROGRAM_BEGIN;
	c->status = STATUS_NORMAL;
	c->delay_timer = 0;
	c->sound_timer = 0;
	c->sp = 0;
	c->current_instruction = 0;
	c->index = 0;

	c->draw_flag = TRUE;
	
	int i;

	for(i=0; i<4096; i++)
		c->memory[i] = 0;

	for(i=0; i<16; i++)
		c->registers[i] = c->stack[i] = c->vregs[i] = c->key[i] = 0;

	return c;
}

void freeChip(Chip c){
	free(c);
}

void fetchAndDecode(Chip c){
	c->current_instruction = c->memory[c->pc] << 8 | c->memory[c->pc+1];
}

void execute(Chip c){
	uc keyPress = FALSE;
	
	// Credit to Lawrence Muller for all the instructions below
	switch(c->current_instruction & 0xF000){		
		case 0x0000:
			switch(c->current_instruction & 0x000F){
				case 0x0000: 
					for(int i = 0; i < 2048; ++i)
						c->display[i] = 0x0;
					c->draw_flag = TRUE;
					c->pc += 2;
					break;

				case 0x000E: 
					c->sp--;			
					c->pc = c->stack[c->sp];	
					c->pc += 2;		
					break;

				default:
					printf ("Invalid Instruction! [0x0000]: 0x%X\n", c->current_instruction);					
			}
			break;

		case 0x1000: 
			c->pc = c->current_instruction & 0x0FFF;
			break;

		case 0x2000: 
			c->stack[c->sp] = c->pc;			
			c->sp++;					
			c->pc = c->current_instruction & 0x0FFF;	
			break;
		
		case 0x3000: 
			if(c->vregs[(c->current_instruction & 0x0F00) >> 8] == (c->current_instruction & 0x00FF))
				c->pc += 4;
			else
				c->pc += 2;
			break;
		
		case 0x4000: 
			if(c->vregs[(c->current_instruction & 0x0F00) >> 8] != (c->current_instruction & 0x00FF))
				c->pc += 4;
			else
				c->pc += 2;
			break;
		
		case 0x5000: 
			if(c->vregs[(c->current_instruction & 0x0F00) >> 8] == c->vregs[(c->current_instruction & 0x00F0) >> 4])
				c->pc += 4;
			else
				c->pc += 2;
			break;
		
		case 0x6000: 
			c->vregs[(c->current_instruction & 0x0F00) >> 8] = c->current_instruction & 0x00FF;
			c->pc += 2;
			break;
		
		case 0x7000: 
			c->vregs[(c->current_instruction & 0x0F00) >> 8] += c->current_instruction & 0x00FF;
			c->pc += 2;
			break;
		
		case 0x8000:
			switch(c->current_instruction & 0x000F){
				case 0x0000: 
					c->vregs[(c->current_instruction & 0x0F00) >> 8] = c->vregs[(c->current_instruction & 0x00F0) >> 4];
					c->pc += 2;
					break;

				case 0x0001: 
					c->vregs[(c->current_instruction & 0x0F00) >> 8] |= c->vregs[(c->current_instruction & 0x00F0) >> 4];
					c->pc += 2;
					break;

				case 0x0002: 
					c->vregs[(c->current_instruction & 0x0F00) >> 8] &= c->vregs[(c->current_instruction & 0x00F0) >> 4];
					c->pc += 2;
					break;

				case 0x0003: 
					c->vregs[(c->current_instruction & 0x0F00) >> 8] ^= c->vregs[(c->current_instruction & 0x00F0) >> 4];
					c->pc += 2;
					break;

				case 0x0004: 
					if(c->vregs[(c->current_instruction & 0x00F0) >> 4] > (0xFF - c->vregs[(c->current_instruction & 0x0F00) >> 8])) 
						c->vregs[0xF] = 1; 
					else 
						c->vregs[0xF] = 0;					
					c->vregs[(c->current_instruction & 0x0F00) >> 8] += c->vregs[(c->current_instruction & 0x00F0) >> 4];
					c->pc += 2;					
					break;

				case 0x0005: 
					if(c->vregs[(c->current_instruction & 0x00F0) >> 4] > c->vregs[(c->current_instruction & 0x0F00) >> 8]) 
						c->vregs[0xF] = 0; 
					else 
						c->vregs[0xF] = 1;					
					c->vregs[(c->current_instruction & 0x0F00) >> 8] -= c->vregs[(c->current_instruction & 0x00F0) >> 4];
					c->pc += 2;
					break;

				case 0x0006: 
					c->vregs[0xF] = c->vregs[(c->current_instruction & 0x0F00) >> 8] & 0x1;
					c->vregs[(c->current_instruction & 0x0F00) >> 8] >>= 1;
					c->pc += 2;
					break;

				case 0x0007: 
					if(c->vregs[(c->current_instruction & 0x0F00) >> 8] > c->vregs[(c->current_instruction & 0x00F0) >> 4])	
						c->vregs[0xF] = 0; 
					else
						c->vregs[0xF] = 1;
					c->vregs[(c->current_instruction & 0x0F00) >> 8] = c->vregs[(c->current_instruction & 0x00F0) >> 4] - c->vregs[(c->current_instruction & 0x0F00) >> 8];				
					c->pc += 2;
					break;

				case 0x000E: 
					c->vregs[0xF] = c->vregs[(c->current_instruction & 0x0F00) >> 8] >> 7;
					c->vregs[(c->current_instruction & 0x0F00) >> 8] <<= 1;
					c->pc += 2;
					break;

				default:
					printf ("Invalid Instruction! [0x8000]: 0x%X\n", c->current_instruction);
			}
			break;
		
		case 0x9000:
			if(c->vregs[(c->current_instruction & 0x0F00) >> 8] != c->vregs[(c->current_instruction & 0x00F0) >> 4])
				c->pc += 4;
			else
				c->pc += 2;
			break;

		case 0xA000:
			c->index = c->current_instruction & 0x0FFF;
			c->pc += 2;
			break;
		
		case 0xB000:
			c->pc = (c->current_instruction & 0x0FFF) + c->vregs[0];
			break;
		
		case 0xC000:
			c->vregs[(c->current_instruction & 0x0F00) >> 8] = (rand() % 0xFF) & (c->current_instruction & 0x00FF);
			c->pc += 2;
			break;
	
		case 0xD000:{
			us x = c->vregs[(c->current_instruction & 0x0F00) >> 8];
			us y = c->vregs[(c->current_instruction & 0x00F0) >> 4];
			us height = c->current_instruction & 0x000F;
			us pixel;

			c->vregs[0xF] = 0;
			for (int yline = 0; yline < height; yline++){
				pixel = c->memory[c->index + yline];
				for(int xline = 0; xline < 8; xline++){
					if((pixel & (0x80 >> xline)) != 0){
						if(c->display[(x + xline + ((y + yline) * 64))] == 1)
							c->vregs[0xF] = 1;  

						c->display[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}
						
			c->draw_flag = TRUE;			
			c->pc += 2;
			break;
		}
			
		case 0xE000:
			switch(c->current_instruction & 0x00FF){
				case 0x009E: 
					if(c->key[c->vregs[(c->current_instruction & 0x0F00) >> 8]] != 0)
						c->pc += 4;
					else
						c->pc += 2;
					break;
				
				case 0x00A1: 
					if(c->key[c->vregs[(c->current_instruction & 0x0F00) >> 8]] == 0)
						c->pc += 4;
					else
						c->pc += 2;
					break;

				default:
					printf ("Invalid Instruction! [0xE000]: 0x%X\n", c->current_instruction);
			}
			break;
		
		case 0xF000:
			switch(c->current_instruction & 0x00FF){
				case 0x0007: 
					c->vregs[(c->current_instruction & 0x0F00) >> 8] = c->delay_timer;
					c->pc += 2;
					break;
								
				case 0x000A:
					for(int i = 0; i < 16; ++i){
						if(c->key[i] != 0){
							c->vregs[(c->current_instruction & 0x0F00) >> 8] = i;
							keyPress = TRUE;
						}
					}

					if(!keyPress) return;
					c->pc += 2;
					break;
				
				case 0x0015: 
					c->delay_timer = c->vregs[(c->current_instruction & 0x0F00) >> 8];
					c->pc += 2;
					break;

				case 0x0018: 
					c->sound_timer = c->vregs[(c->current_instruction & 0x0F00) >> 8];
					c->pc += 2;
					break;

				case 0x001E: 
					if(c->index + c->vregs[(c->current_instruction & 0x0F00) >> 8] > 0xFFF)	
						c->vregs[0xF] = 1;
					else
						c->vregs[0xF] = 0;
					c->index += c->vregs[(c->current_instruction & 0x0F00) >> 8];
					c->pc += 2;
					break;

				case 0x0029: 
					c->index = c->vregs[(c->current_instruction & 0x0F00) >> 8] * 0x5;
					c->pc += 2;
					break;

				case 0x0033: 
					c->memory[c->index]     = c->vregs[(c->current_instruction & 0x0F00) >> 8] / 100;
					c->memory[c->index + 1] = (c->vregs[(c->current_instruction & 0x0F00) >> 8] / 10) % 10;
					c->memory[c->index + 2] = (c->vregs[(c->current_instruction & 0x0F00) >> 8] % 100) % 10;					
					c->pc += 2;
					break;

				case 0x0055: 
					for (int i = 0; i <= ((c->current_instruction & 0x0F00) >> 8); ++i)
						c->memory[c->index + i] = c->vregs[i];	

					
					c->index += ((c->current_instruction & 0x0F00) >> 8) + 1;
					c->pc += 2;
					break;

				case 0x0065: 
					for (int i = 0; i <= ((c->current_instruction & 0x0F00) >> 8); ++i)
						c->vregs[i] = c->memory[c->index + i];			

					
					c->index += ((c->current_instruction & 0x0F00) >> 8) + 1;
					c->pc += 2;
					break;

				default:
					printf ("Invalid Instruction! [0xF000]: 0x%X\n", c->current_instruction);
			}
			break;

		default:
			printf ("Invalid Instruction!: 0x%X\n", c->current_instruction);
	}
}


void cycle(Chip c){
	fetchAndDecode(c);
	execute(c);

	if(c->delay_timer > 0) --c->delay_timer;
	if(c->sound_timer == 1) printf("\a");
	if(c->sound_timer > 0) --c->sound_timer;
}

void loadProgram(Chip c, char * filename){
	FILE * program = fopen(filename, "rb");
	if(program==NULL){
		fprintf(stderr, "File Not Found!\n");
		exit(0);
	}

	fseek(program, 0, SEEK_END);
	us lSize = ftell(program);
	rewind(program);

	if(fread(c->memory+MAP_PROGRAM_BEGIN, 1, lSize, program) != lSize){
		fprintf(stderr, "Reading Error!\n");
		abort();
	}
	printf("Program Size: %d\n",lSize);
	fclose(program);
}