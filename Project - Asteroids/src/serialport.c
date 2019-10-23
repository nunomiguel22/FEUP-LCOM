#include "serialport.h"
#include <stdio.h>
#include "macros.h"

static int com1_hook_id = 0;
static int com2_hook_id = 5;

uint8_t sp_control (){
	uint32_t conf;
	sys_inb( (COM1_ADDR + SP_LINE_CTRL), &conf );
	return conf;
}

uint8_t sp_status (){
	uint32_t lsr;
	sys_inb( (COM1_ADDR + SP_LINE_STATUS), &lsr );
	return lsr;
}

void sp_write_th (uint16_t COM_ADDR, uint32_t cdata){
	sys_outb(COM_ADDR, cdata);
}

void sp_read_buffer (uint16_t COM_ADDR, uint8_t *cdata){
	uint32_t temp;
	sys_inb(COM_ADDR, &temp);
	*cdata = temp;
	
}

int sp_subscribe_int (){
	
	sys_irqsetpolicy(serial_com1_irq_line, IRQ_REENABLE | IRQ_EXCLUSIVE, &com1_hook_id);
	sys_irqsetpolicy(serial_com2_irq_line, IRQ_REENABLE | IRQ_EXCLUSIVE, &com2_hook_id); 	
	return 0;
}
	
int sp_unsubscribe_int(){

	sys_irqrmpolicy(&com1_hook_id);
	sys_irqrmpolicy(&com2_hook_id);
	return 0;
}

void sp_enable_int(){
	
	uint32_t cdata;
	sys_inb(COM1_ADDR + SP_IH_ENABLE, &cdata);
	cdata |= 0x03;
	sys_outb(COM1_ADDR + SP_IH_ENABLE, cdata);
	
	sys_inb(COM2_ADDR + SP_IH_ENABLE, &cdata);
	cdata |= 0x03;
	sys_outb(COM2_ADDR + SP_IH_ENABLE, cdata);
}

void sp_disable_int(){
	
	uint32_t cdata;
	sys_inb(COM1_ADDR + 1, &cdata);
	cdata &= 0xFE;
	sys_outb(COM1_ADDR + 1, cdata);
	
	sys_inb(COM2_ADDR + 1, &cdata);
	cdata &= 0xFE;
	sys_outb(COM2_ADDR + 1, cdata);
}

bool sp_transmitter_ready(uint16_t COM_ADDR){
	
	uint32_t cdata;
	sys_inb( (COM_ADDR + SP_INT_IDENTIFICATION), &cdata);
	
	uint8_t origin = (cdata & SP_INT_ORIGIN);

	if (origin == 2)
		return true;
	
	return false;
}

bool sp_data_available(uint16_t COM_ADDR){
	
	uint32_t cdata;
	sys_inb( (COM_ADDR + SP_INT_IDENTIFICATION), &cdata);
	uint8_t origin = (cdata & SP_INT_ORIGIN);

	if (origin == 4)
		return true;
	
	return false;
}


void sp_set_rate(uint16_t COM_ADDR, uint32_t rate){
	
	uint32_t lcr = 0;
	uint32_t outlcr = 0;
	uint32_t divisorl = SP_RATE / rate;
	
	sys_inb( (COM_ADDR + SP_LINE_CTRL) , &lcr);
	outlcr = lcr | DLAB;
	sys_outb( (COM_ADDR + SP_LINE_CTRL ), outlcr);
	
	sys_outb ((COM_ADDR + DL_LSB ), (divisorl & 0xFF));
	sys_outb ((COM_ADDR + DL_MSB ), (divisorl >> 8));
			 
	sys_outb( (COM_ADDR + SP_LINE_CTRL ), lcr);
}

void sp_print_config (){
	uint32_t lcr;
	uint32_t IER;
	uint32_t outlcr;
	uint32_t msb, lsb, rate;
	
	sys_inb( (COM1_ADDR + SP_LINE_CTRL) , &lcr);
	printf("\nLINE CTRL REG:%X\n", lcr);
		
	sys_inb(COM1_ADDR + SP_IH_ENABLE, &IER);
	printf("INTERRUPT ENABLE REG:%X\n", IER);
	
	outlcr = lcr | DLAB;
	sys_outb( (COM1_ADDR + SP_LINE_CTRL ), outlcr);
	sys_inb ((COM1_ADDR + DL_LSB ), &lsb);
	sys_inb ((COM1_ADDR + DL_MSB ), &msb);
	rate =  msb << 8;
	rate += lsb;
	printf("DIVISOR LATCH:%X  RATE:%d\n", rate, SP_RATE/rate);
	outlcr = 0;
	
	sys_inb( (COM1_ADDR + SP_LINE_CTRL) , &outlcr);
	printf("LINE CTRL REG:%X\n", outlcr);
	
	sys_outb( (COM1_ADDR + SP_LINE_CTRL ), lcr);
	sys_inb( (COM1_ADDR + SP_LINE_CTRL) , &outlcr);
	printf("LINE CTRL REG:%X\n", outlcr);
}

void sp_initialize_fifo(uint16_t COM_ADDR){
	uint32_t fifreg;
	sys_inb( (COM_ADDR + SP_FIFO_CONTROL) , &fifreg);
	fifreg |= (BIT(0) | BIT(1) | BIT(2));
	fifreg &= ~(BIT(7) | BIT(6));
	sys_outb( (COM_ADDR + SP_FIFO_CONTROL), fifreg);
}


