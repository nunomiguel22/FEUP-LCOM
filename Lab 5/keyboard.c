#include <minix/sysutil.h>
#include "keyboard.h"
#include "macros.h"


extern uint32_t code;
uint32_t code = 0;

int (keyboard_subscribe_int)(uint8_t *bit_no) {
 
 int hook_id = 3;
 
 sys_irqsetpolicy(kb_irq_line, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id);
 
 *bit_no = hook_id;

  return *bit_no;
}

int (keyboard_unsubscribe_int)(uint8_t *bit_no) 
{
	int hook_id = *bit_no;
	sys_irqrmpolicy (&hook_id);

  return 1;
}


int (verify_stat_error)(){

	uint32_t stat;
	
	sys_inb (STAT_REG, &stat);
	
	int c = 0;
	while(c <= 100 && ( stat & (PAR_ERR | TO_ERR) ) != 0 && ( stat & 0x20) != 0)
	{
			tickdelay(micros_to_ticks(DELAY_US));
			c++;
	}
	
	if(c==100)
		return 1;
	
	return 0;
	
}

int (kbc_issue_cmd)(uint8_t cmd) {
	uint32_t stat;
	
	unsigned int i;
	
	sys_inb (STAT_REG, &stat);
	
	for (i = 0; i <= 100; i++){
		
		if ( (stat & IBF) != 0 ) {		
		sys_inb (STAT_REG, &stat);
		}
		else break;
		
		tickdelay(micros_to_ticks(DELAY_US));
	}
	
	if (i == 100)
		return 1;
	
	sys_outb (STAT_REG, cmd);
	
	return 0;
}

int (kbc_write_argument)(uint32_t arg){
	
	uint32_t stat;
	unsigned int i;
	
	sys_inb(STAT_REG, &stat);
	
	for (i = 0; i <= 100; i++){
	
		if ( (stat & OBF) != 0 ) {	
			sys_inb (STAT_REG, &stat);
		}
		else break;
		
		tickdelay(micros_to_ticks(DELAY_US));
	}
	
	if (i == 100)
		return 1;
	
	sys_outb(OUT_BUF, arg);
	
	return 0;
}

int (kbc_read_return) (uint32_t *arg){
	
	unsigned int i;
	uint32_t stat;
	
	sys_inb(STAT_REG, &stat);

	
	for (i = 0; i <= 100; i++) {
		
		if ( (stat & OBF) == 0 ) {
			sys_inb (STAT_REG, &stat);
		}
		else break;
		tickdelay(micros_to_ticks(DELAY_US));
	}
	
	sys_inb (OUT_BUF, arg);
	
	if ( (stat & (PAR_ERR | TO_ERR) ) == 0 && i < 100)
		 return 0;
	else return 1;
	
}

void (default_kbc)()
{
	uint8_t dflt = minix_get_dflt_kbc_cmd_byte();
	kbc_issue_cmd(kbc_write_cmd); 
	kbc_write_argument(dflt);
}
