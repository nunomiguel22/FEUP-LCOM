#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

#include <minix/syslib.h>

#include "macros.h"

extern unsigned int timerTickCounter;
unsigned int timerTickCounter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  
  uint8_t status = 0x00;
  uint8_t timerAddress;
  uint8_t control;
  //1234DD sendo o valor do clock em hexadecimal, counter sendo o valor a ser passado para o endereço do timer
  uint16_t counter = 0x1234DD / freq;
  uint8_t lsb; 
  uint8_t msb;
  timer_get_conf(timer, &status);
  
  //guarda os 4 lsb
  uint8_t lsb4 = status & 0x0F; 
  
  /*isola o lsb e msb do counter 
  IMP: funçoes util_get_LSB e util_get_MSB implementadas no lab2.c*/
  lsb = 0xff & counter;
  msb = counter >> 8;
  
  //seleciona o timer e prepara a control word
  switch (timer)
  {
	case 0x00:
		timerAddress = TIMER_0;
		control = 0x30 | lsb4;
		break;
	case 0x01:
		timerAddress = TIMER_1;
		control = 0x70 | lsb4;
		break;
	case 0x02:
		timerAddress = TIMER_2;
		control = 0xB0 | lsb4;
		break;
  }
  
  //escrever o control
  sys_outb(TIMER_CTRL, control);
  
  //escrever lsb seguido de msb para o timer
  sys_outb(timerAddress, lsb);
  sys_outb(timerAddress, msb);
  
  return 1;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
 
 int hook_id = 1;
 
 sys_irqsetpolicy(timer_irq_line, IRQ_REENABLE, &hook_id);
 
 *bit_no = hook_id;

  return *bit_no;
}

int (timer_unsub_int)(uint8_t *bit_no) 
{
int hook_id = *bit_no;
sys_irqrmpolicy (&hook_id);

  return 0;
}

void (timer_int_handler)() {
  	timerTickCounter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
	
uint32_t *statusByte; //Como sys_inb usa 32 bits, o status vai ser copiado para este local de memoria primeiro 
uint8_t readBackCommand; //Comando de readback a carregar no control register
uint8_t timerAddress; //Endereço de memoria do timer a usar
uint8_t controlRegister = TIMER_CTRL; //Endereço de memoria do registo de controlo

//Alocar 4 bytes de memoria para receber o status 
statusByte = (uint32_t*) malloc (4); 

//Atribuir o comando de read back e endereço do timer de acordo com o timer escolhido
switch (timer){
	case 0x00:
		readBackCommand =	0xE2;
		timerAddress = TIMER_0;
		break;
	case 0x01:
		readBackCommand = 0xE4;
		timerAddress = TIMER_1;
		break;
	case 0x02:
		readBackCommand = 0xE8;
		timerAddress = TIMER_2;	
		break;
}
 
//Read-Back escrito no Control Register. Coloca o status byte no endereço do timer 
sys_outb(controlRegister, readBackCommand); 

//Escrever o status byte na variavel temporaria
sys_inb(timerAddress, statusByte); 

//Passar a variavel temporaria em uint32_t para o destino final uint8_t
*st = *statusByte; 

  return 1;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
							
							union timer_status_field_val status;
							
							
							//Coloca o valor correto (dependo do field) na uniao a ser usada na funçao de print 
							switch (field){
								//field = all
								case 0:
										status.byte = st;
										break;
								//field = init
								case 1:
										if (st & 0x10 && st & 0x20)
											status.in_mode = 3;
										else if 	(st & 0x10)
											status.in_mode = 1;
											else if (st & 0x20)
													status.in_mode = 2;
												else status.in_mode = 0;
										break;
								//field = mode
								case 2:
										if (st & 0x02 && st & 0x08)
												status.count_mode = 0x05;
										else if (st & 0x08)
													status.count_mode = 0x04;
											else if (st & 0x02 && st & 0x04)
														status.count_mode = 0x03;
												else if (st & 0x04)
															status.count_mode = 0x02;
													else if (st &0x02)
																status.count_mode = 0x01;
														else status.count_mode = 0x00;
										break;
								//field = base;
								case 3:
										if (st & 0x01)
											status.bcd = true;		
										else
											status.bcd = false;	
										break;			
							}
							
							//Funçao print pre-feita
							timer_print_config(timer, field, status);
							
 return 1;
}
