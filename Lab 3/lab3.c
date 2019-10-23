#include <lcom/lcf.h>
#include "keyboard.h"
#include <lcom/timer.h>

extern uint32_t code;
uint32_t code = 0;
	//Stores sys_inb_cnt counter value
extern int inbCounter;
int inbCounter = 0;

extern uint8_t *hook_id_timer;
uint8_t *hook_id_timer;

//to count the timer 0 interrupts
extern int irqCounter;

int main(int argc, char *argv[]) {
	
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int (kbd_test_scan)(bool assembly) {
	
	//Subscribes to keyboard interupts using hook_id
	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	keyboard_subscribe_int (hook_id);
	
	int ipc_status;
	message msg;
	
	//Flag for makecodes with two bytes
	bool twoBytes = false;
	
	//irq_set to the input value of hook_id value in keyboard_subscribe_int
	uint32_t irq_set = 0x00000004;
	

	
	while(code != 0x81)  {
		
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		
		if (is_ipc_notify(ipc_status)) { /* received notification */
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */	
						
					if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
					
						if(!assembly)
							kbc_ih();
						
						else kbc_asm_ih();
						
						//Two byte makecodes start with E0
						if (code == 0xE0)
							twoBytes = true;
						
						//Breakcode when over 0x80, otherwise its a makecode
						bool isMake = true;
						if (code != 0xE0 && code >= 0x80){
							isMake = false;
						}
						//Print calls for single and double byte codes
						if (twoBytes && code != 0xE0)
						{
							uint8_t bytes[ ] = {0xE0, code};
							kbd_print_scancode(isMake, 2, bytes);
							twoBytes = false;
							
						} else if (!twoBytes)
						{
							uint8_t bytes[ ] = {code};
							kbd_print_scancode(isMake, 1, bytes);
						}
						

					}
                break;
				default:
					break; /* no other notifications expected: do nothing */	
			}
		}
	}
	
	//Print call for number of times the program read from OUT_BUF
	if (!assembly)
		kbd_print_no_sysinb(inbCounter);
	
	//Unsubscribes from the keyboard interrupts
	keyboard_unsubscribe_int(hook_id);
	
	
	return 0;
}

int (kbd_test_poll)() {
	//Code will receive the values from OUT_BUF, set to 0 to start so it enters the first cycle
	uint32_t code = 0x00000000;
	uint8_t STAT_REG = 0x64;
	uint32_t stat;
	//Flag for makecodes with two bytes
	bool twoBytes = false;
	uint8_t aux = 0x20;
	uint8_t OBF = 0x01;
	
	//OUT_BUF register address
	uint8_t OUT_BUF = 0x60;
	int inbCounter;	
	//uint8_t cmdbyte;
		
    while(code != 0x81){
		
		inbCounter = sys_inb_cnt (STAT_REG, &stat);

		if ( (stat & OBF ) && ((stat & aux)  == 0)){
			
			if(verify_stat_error() != 0)
				return 1;
		
			inbCounter = sys_inb_cnt (OUT_BUF, &code);

			//Two byte makecodes start with E0
			if (code == 0xE0)
				twoBytes = true;
			
			//Breakcode when over 0x80, otherwise its a makecode
			bool isMake = true;
			if (code != 0xE0 && code >= 0x80)
				isMake = false;
						
			//Print calls for single and double byte codes
			if (twoBytes && code != 0xE0){
							
							uint8_t bytes[ ] = {0xE0, code};
							kbd_print_scancode(isMake, 2, bytes);
							twoBytes = false;
			}
			else if (!twoBytes){
				
							uint8_t bytes[ ] = {code};
							kbd_print_scancode(isMake, 1, bytes);
					}
		}
	}
	
	//Print call for number of times the program read from OUT_BUF
	kbd_print_no_sysinb(inbCounter);

	kbc_toggle_keyboard_interrupts ();
	
	return 0;
}


int (kbd_test_timed_scan)(uint8_t n){

	//subscribe kbd interrupts
	uint8_t *hook_id_kbd;
	hook_id_kbd = (uint8_t*) malloc (1);
	keyboard_subscribe_int (hook_id_kbd);
	
	//subscribe timer0 interrupts
	
	hook_id_timer = (uint8_t*) malloc (1);
	timer_subscribe_int (hook_id_timer);	
	
	
	uint32_t irq_kbd = 0x00000004;
	uint32_t irq_timer0 = 0x00000002;
	
	int ipc_status;
	message msg;
	
	bool twoBytes = false;

	while ( (irqCounter / 60 < n) && (code != 0x81) )
	{
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) 
		{ 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		
		if (is_ipc_notify(ipc_status)) 
		{ 
			switch (_ENDPOINT_P(msg.m_source)) 
			{
				case HARDWARE: /*hardware interrupt notification*/
					if (msg.m_notify.interrupts & irq_timer0) /*subscribed interrupt* for timer 0*/
					{
						//timer 0 process
						timer_int_handler();
					}
					if (msg.m_notify.interrupts & irq_kbd) /*subscribed interrupt for keyboard*/
					{
						//kbd process
						kbc_ih();
						irqCounter = 0;
						//Two byte makecodes start with E0
						if (code == 0xE0)
							twoBytes = true;
						
						//Breakcode when over 0x80, otherwise its a makecode
						bool isMake = true;
						if (code != 0xE0 && code >= 0x80){
							isMake = false;
						}
						//Print calls for single and double byte codes
						if (twoBytes && code != 0xE0)
						{
							uint8_t bytes[ ] = {0xE0, code};
							kbd_print_scancode(isMake, 2, bytes);
							twoBytes = false;
							
						} else if (!twoBytes)
						{
							uint8_t bytes[ ] = {code};
							kbd_print_scancode(isMake, 1, bytes);
						}
					
					}
					break;
					
				default:
					break;
			}
		}
	}
	
	if (code == 0x81)
		printf("\n User finished! \n");
	
	if (irqCounter / 60 >= n)
		printf("\n Time's up! \n");
	
	//unsubcribes
	keyboard_unsubscribe_int(hook_id_kbd);
	timer_unsubscribe_int();

	return 0;
}


