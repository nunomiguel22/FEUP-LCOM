#include <minix/sysutil.h>
#include "keyboard.h"
#define DELAY_US 20000




int (keyboard_subscribe_int)(uint8_t *bit_no) {
 
 int hook_id = 2;
 
 sys_irqsetpolicy(1, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id);
 
 *bit_no = hook_id;

  return *bit_no;
}

int (keyboard_unsubscribe_int)(uint8_t *bit_no) 
{
	int hook_id = *bit_no;
	sys_irqrmpolicy (&hook_id);

  return 1;
}

int (sys_inb_cnt)(port_t port, uint32_t *byte) {//DUVIDAS: unsigned long e return, tickdelay
	static int cnt = 0;
	sys_inb(port, byte);
	cnt++;
	return cnt;
}

int (kbc_issue_cmd)(uint8_t cmd){//escrever comandos no status register
	
	uint8_t STAT_REG = 0x64;
	uint8_t KBC_CMD_REG = 0x64;
	uint32_t stat;
	uint8_t IBF = 0x01;
	
	bool isWritten = false;
	while( !isWritten ){//cond passa a falso se escrever o cmd
		
		sys_inb (STAT_REG, &stat);
		if ( (stat & IBF) == 0){
			sys_outb (KBC_CMD_REG, cmd);
			isWritten = true;
			return 0;
		}
	}
	
	return 1;
}

int (verify_stat_error)()
{
	uint8_t STAT_REG = 0x64;
	uint32_t stat;
	uint8_t PAR_ERR = 0x80;
	uint8_t TO_ERR = 0x40;
	 
	 sys_inb (STAT_REG, &stat);
	
	int c = 0;
	while(c <= 100 && ( stat & (PAR_ERR | TO_ERR) ) != 0)
	{
			tickdelay(micros_to_ticks(DELAY_US));
			c++;
	}
	
	if(c==100)
		return 1;
	
	return 0;
	
}

uint32_t (kbc_read_command)(){
	uint8_t cmd = 0x20;
	kbc_issue_cmd(cmd);
	
	uint8_t OUT_BUF = 0x60;
	uint32_t cmdWord;
	sys_inb(OUT_BUF, &cmdWord);
	
	return cmdWord;
}

void (kbc_write_argument)(uint32_t arg){
	uint8_t OUT_BUF = 0x60;
	sys_outb(OUT_BUF, arg);
}

void (kbc_write_command)(uint32_t arg){
	uint32_t cmd = 0x60;
	kbc_issue_cmd(cmd);
	kbc_write_argument(arg);
}

uint32_t (kbc_self_test)(){
	uint32_t cmd = 0xAA;
	uint32_t OUT_BUF = 0x60;
	uint32_t data;
	
	kbc_issue_cmd(cmd);
	sys_inb (OUT_BUF, &data);
	
	return data;
	}

uint32_t (kbc_check_keyboard_interface)(){
	uint32_t cmd = 0xAB;
	uint32_t OUT_BUF = 0x60;
	uint32_t data;
	kbc_issue_cmd(cmd);
	sys_inb (OUT_BUF, &data);
	return data;
}

uint32_t (kbc_read_status)(){
	uint8_t STAT_REG = 0x64;
	uint32_t stat;
	sys_inb(STAT_REG, &stat);
	return stat;
}

void (kbc_toggle_keyboard_interrupts) (){
	uint8_t cmdbyte;
	cmdbyte = kbc_read_command();
	cmdbyte ^= 1 << 0;
	kbc_write_command(cmdbyte);
}

