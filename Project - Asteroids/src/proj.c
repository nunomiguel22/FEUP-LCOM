#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <machine/asm.h>

// Any header files included below this line should have been created by you
#include "gamestate.h"
#include "devices.h"
#include "macros.h"
#include "vcard.h"

int main(int argc, char *argv[]) {
	srand(time(NULL));
	lcf_set_language("EN-US");
	if (lcf_start(argc, argv))
		return 1;
	lcf_cleanup();

	return 0;
}

int (proj_main_loop)(int argc, char *argv[]) {
	
	if (argc != 1){
		printf("	lcom_run proj \"Path to project folder\"\n");
		printf("	lcom_run proj \"default\" for /home/lcom/labs/proj/\n");
		return 1;
	}
	
	/* Enables I/O operations */
	sys_enable_iop(SELF);

	/* Initiate game data */
	game_data game;
	
	if (!strcmp(argv[0], "default"))
		strncpy(game.FILEPATH, "/home/lcom/labs/proj/src/", 50);
	else strncpy(game.FILEPATH, argv[0], 50);
	
	game_data_init(&game);
	
	/* Initiate graphics mode */
	game.gr_buffer = vmode_init(Direct24_1024); 
	
	show_splash(&game);

	/* Initiate devices */
	init_devices();
	
	/* Game Loop */
	interrupt_handler(&game);
	
	/* Reset Devices and initiate text mode */
	reset_devices();
	vmode_exit();
	
  return 0;
}
