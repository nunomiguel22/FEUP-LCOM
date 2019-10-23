#include <lcom/lcf.h>

#include <lcom/lab2.h>
#include <lcom/timer.h>

#include <stdbool.h>
#include <stdint.h>

#include <minix/syslib.h>

extern int irqCounter; 


int main(int argc, char *argv[]) {


  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
	
	uint8_t *statusByte; //pointer to address receiving the status byte
	
	//allocate memory for status byte
	statusByte = (uint8_t*) malloc(1);
	
	//calls function that will write the status byte to our pointer
	timer_get_conf(timer, statusByte); 
	
	//calls function to print the status byte
	timer_display_conf (timer, *statusByte, field);
	
  return 1;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  
  
  timer_set_frequency(timer, freq);

  return 1;
}

int(timer_test_int)(uint8_t time) {

	//declare and allocate memory for the hook_id (byte returned by the sys_irqsetpolicy function)
	uint8_t *hook_id; 
	hook_id = (uint8_t*) malloc (1);
	
	//calls function that will subscribe to the notifications   
	timer_subscribe_int (hook_id); 
		
	int ipc_status;
	message msg;
	uint8_t irq_set = 0x02; // Hook_id flipped bit 1 in timer_subscribe_int meaning 0010 = 2, for the correct notifications;

	while(irqCounter / 60 < time)  {
	
		int r = driver_receive(ANY, &msg, &ipc_status); //r within IF was causing compiling errors, so put it outside
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */	
					if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
					
						//increments the interrupt counter and prints elapsed time message when appropriate
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
