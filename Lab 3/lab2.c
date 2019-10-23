#include <lcom/lcf.h>

#include <lcom/lab2.h>
#include <lcom/timer.h>

#include <stdbool.h>
#include <stdint.h>

#include <minix/syslib.h>

extern int irqCounter; 

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
	
	uint8_t *statusByte; //Pointer para o endereço que vai receber o status byte
	
	//Alocar 1 byte de memoria para receber o status
	statusByte = (uint8_t*) malloc(1);
	
	//Coloca a configuraçao do timer em statusByte
	timer_get_conf(timer, statusByte); 
    
	//DEBUG
	//printf ("Full byte test = %x\n" , *statusByte); 
	
	timer_display_conf (timer, *statusByte, field);
	
  return 1;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  
  
  timer_set_frequency(timer, freq);

  return 1;
}

int(timer_test_int)(uint8_t time) {

	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	*hook_id = 0;
	
	timer_subscribe_int (hook_id);
		
	int ipc_status;
	message msg;
	uint32_t irq_set = 0x00000002; // Tem que ser igual a 2, mas qual a origem?

	while(irqCounter / 60 < time)  {
	
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */	
						
					if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
						timer_int_handler();
						if (irqCounter % 60 == 0){
							timer_print_elapsed_time();
						}
					}
                break;
				default:
					break; /* no other notifications expected: do nothing */	
			}
		}
	}

	timer_unsubscribe_int();
	return 1;
}

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  
	*lsb = val & 0xFF;
	
	if(lsb != 0x00)
		return 0;
	
	return 1;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  
	* msb = val >>  8;
	
	
	if(msb != 0x00)
		return 0;

	return 1;
}
