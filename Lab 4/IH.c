#include "mouse.h"
#include "macros.h"

extern uint32_t code;

void (mouse_ih)(){
	
	if(verify_stat_error() == 0){
		 sys_inb(OUT_BUF, &code);
	}
}


