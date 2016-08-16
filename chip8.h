/*
	chirp - CHIP-8 Machine Emulator
	Written by Riyasat Saber
*/

#define STATUS_NORMAL 		0
#define STATUS_ERROR      	1
#define STATUS_HALT			2

#define MAP_FONTSET_BEGIN 	0x050
#define MAP_PROGRAM_BEGIN 	0x200

#define TRUE				1
#define FALSE				0

#define ON					1
#define OFF					0

typedef unsigned short us;
typedef unsigned char uc;

typedef struct _chip{
	uc memory[4096];		// 4K of Memory
	uc registers[16];		// 16 Registers

	uc display[64*32];		// 64x32 Monochrome Display
	uc key[16];				// Input keys for current cycle, multiple key strokes can be detected

	us stack[16];			// Stack, used to store pc before jump
	us sp;					// Stack index

	uc delay_timer;			// Timers, when non-zero will count down
	uc sound_timer;			// Count down at 60Hz

	us current_instruction;	// Decoded instruction

	uc draw_flag;			// Used to notify a redraw

	us index;				// Index register
	us pc;					// Program Counter

	uc status;				// System status, unused at this stage

	uc vregs[16];			
}*Chip;

Chip newChip();
void freeChip(Chip c);
void fetchAndDecode(Chip c);
void execute(Chip c);
void cycle(Chip c);
void loadProgram(Chip c, char * filename);