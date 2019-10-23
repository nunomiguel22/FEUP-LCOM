#include <lcom/lcf.h>
#include "keyboard.h"

#define DELAY_US 20000

extern uint32_t code;
	
	//Stores sys_inb_cnt counter value
extern int inbCounter;

void (kbc_ih)(){//DUVIDA: Deixar enter premido gera breakcode

	//OUT_BUF register address
	uint8_t OUT_BUF = 0x60;
	
	if(verify_stat_error() == 0){
		inbCounter = sys_inb_cnt(OUT_BUF, &code);
	}
}


