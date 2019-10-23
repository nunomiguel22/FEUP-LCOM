// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>
// Any header files included below this line should have been created by you

#include "mouse.h"
#include "macros.h"

extern uint32_t code;
uint32_t code = 0;
extern int irqCounter;
extern uint8_t *hook_id_timer;
uint8_t *hook_id_timer;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}



int (mouse_test_packet)(uint32_t cnt) {
	
	/*
	*  Variables
	*/
	
	int bytecnt = 0;//Keeps track of the packet's byte number
	uint8_t bytes[3];//Stores the packet
	struct packet pp;
	int ipc_status;
	message msg;
		
	
	//Enables the mouse's data reporting
	
	enable_data_reporting();//Function in mouse.h/c

	/*
	*  Mouse interrupt subscription
	*/
	
	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	mouse_subscribe_int (hook_id);

	/*
	*  Driver receive cycle, exits when the specified number of packets is read
	*/
	
	
	while (cnt > 0)
	{
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		
		if (is_ipc_notify(ipc_status)) {
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: 	
						
					if (msg.m_notify.interrupts & irq_mouse) 
					{ 
						//Calls mouse interrupt handler on every mouse interrupt			
						
						mouse_ih();
						
						/*
						* Reads byte 1
						*/
						
						if (bytecnt == 0){
							if (code & byte1_fixed){
								bytes[0] = code;
								bytecnt++;
								}							
							continue;
						}
						
						/*
						* Reads byte 2
						*/
						
						if (bytecnt == 1){
							bytes[1] = code;
							bytecnt++;
							continue;
						}
						
						/*
						* Reads byte 3, parses and prints packet, resets packet cycle
						*/
						
						if (bytecnt == 2){
							bytes[2] = code;
							
							pp = parse_mpacket(bytes[0], bytes[1], bytes[2]);
							mouse_print_packet(&pp);
							
							bytecnt = 0;
							cnt--;
						}
					}
                break;
				default:
					break; 
			}
		}
	}
	
	/*
	*  Unsubscribes mouse interrupts and disables data reporting 
	*/

	mouse_unsubscribe_int(hook_id);
	disable_data_reporting();

    return 0;
}



int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
	
	/*
	*  Variables
	*/
	
	int bytecnt = 0;//Keeps track of the packet's byte number
	uint8_t bytes[3];//Stores the packet
	struct packet pp;
	
	/*
	*  Polling cycle, exits when the specified number of packets is read
	*/

	while (cnt > 0) {
		
		if ( set_data_reading() )
			return 1;
	
		for (int i = 0; i < 3; i++) {
			
			//Reads output buffer
			sys_inb(OUT_BUF, &code);

			/*
			*  Reads byte 1
			*/
						
			if (bytecnt == 0){
				if (code & byte1_fixed){
						bytes[0] = code;
						bytecnt++;
						}						
				}
						
			/*
			*  Reads byte 2
			*/
						
						else if (bytecnt == 1){
							bytes[1] = code;
							bytecnt++;
							}
	
			/*
			* Reads byte 3, parses and prints packet, resets packet counter
			*/
			
						else if (bytecnt == 2){
							bytes[2] = code;
							
							pp = parse_mpacket(bytes[0], bytes[1], bytes[2]);
							mouse_print_packet(&pp); 
							
							bytecnt = 0;
						}
		}
		
			/*
			*  Applies specified delay between packets, decrements packet counter
			*/
			 
		tickdelay(micros_to_ticks(period));
		cnt--;
	}
	
	/*
	*  Sets default kbc's command byte, enables mouse stream mode and disables data reporting 
	*/
	
	default_kbc();
	set_stream_mode();
	disable_data_reporting();
	
    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
	
	/*
	*  Variables
	*/
	
	int bytecnt = 0;//Keeps track of the packet's byte number
	uint8_t bytes[3];//Stores the packet
	struct packet pp;
	int ipc_status;
	message msg;
	
	//Enables the mouse's data reporting
	
	enable_data_reporting();
	
	/*
	*  Mouse and timer interrupt subscription
	*/
	
		//mouse
	uint8_t *hook_id_kbd;
	hook_id_kbd = (uint8_t*) malloc (1);
	mouse_subscribe_int (hook_id_kbd);
	
		//timer 0
	hook_id_timer = (uint8_t*) malloc (1);
	timer_subscribe_int (hook_id_timer);	
	
	/*
	*  Driver receive cycle, exits when idle for specified time
	*/

	while ( ( irqCounter / 60 ) < idle_time)
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
						//calls timer interrupt handler on every timer0 interrupt
						timer_int_handler();
					}
					if (msg.m_notify.interrupts & irq_mouse) /*subscribed interrupt for keyboard*/
					{

						/*
						*  Mouse interrupt handler, resets idle counter every interrupt
						*/
						
						mouse_ih();
						irqCounter = 0;
						
						/*
						*  Reads byte 1
						*/
						
						if (bytecnt == 0){
							if (code & byte1_fixed){
								bytes[0] = code;
								bytecnt++;
								}							
							continue;
						}
						
						/*
						*  Reads byte 2
						*/
						
						if (bytecnt == 1){
							bytes[1] = code;
							bytecnt++;
							continue;
						}
						
						/*
						* Reads byte 3, parses and prints packet, resets packet cycle
						*/
						
						if (bytecnt == 2){
							bytes[2] = code;
							
							pp = parse_mpacket(bytes[0], bytes[1], bytes[2]);//Parses mouse packet
							mouse_print_packet(&pp); //Prints mouse packet
							
							bytecnt = 0;
						}
					}
					break;
					
				default:
					break;
			}
		}
	}
		
	/*
	*  Unsubscribes mouse/timer0 interrupts and disables data reporting 
	*/
	
	mouse_unsubscribe_int(hook_id_kbd);
	timer_unsubscribe_int();
	disable_data_reporting();
	
    return 0;
}


int (mouse_test_gesture) (uint8_t x_len, uint8_t tolerance){
	

	
	/*
	*  Variables
	*/
	struct mouse_ev evt;//mouse event struct
	state_t gest_state = INIT;//Status of state machine
	int bytecnt = 0;//Keeps track of the packet's byte number
	uint8_t bytes[3];//Stores the packet
	struct packet pp;
	int ipc_status;
	message msg;
		
	
	//Enables the mouse's data reporting
	
	enable_data_reporting();//Function in mouse.h/c

	/*
	*  Mouse interrupt subscription
	*/
	
	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	mouse_subscribe_int (hook_id);

	/*
	*  Driver receive cycle, exits when the specified number of packets is read
	*/
	
	while (gest_state != COMP)
	{
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		
		if (is_ipc_notify(ipc_status)) {
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: 	
						
					if (msg.m_notify.interrupts & irq_mouse) 
					{ 
						//Calls mouse interrupt handler on every mouse interrupt			
						
						mouse_ih();
						
						/*
						* Reads byte 1
						*/
						
						if (bytecnt == 0){
							if (code & byte1_fixed){
								bytes[0] = code;
								bytecnt++;
								}							
							continue;
						}
						
						/*
						* Reads byte 2
						*/
						
						if (bytecnt == 1){
							bytes[1] = code;
							bytecnt++;
							continue;
						}
						
						/*
						* Reads byte 3, parses and prints packet, resets packet cycle, updates state machine
						*/
						
						if (bytecnt == 2){
							bytes[2] = code;
							
							pp = parse_mpacket(bytes[0], bytes[1], bytes[2]); //parses mouse packet
							mouse_print_packet(&pp); //Prints mouse packet
							
							evt = m_detect_evt(&pp); //Detects mouse events
							state_machine(&evt, &gest_state, x_len, tolerance); //updates state
							
							if (gest_state == COMP) {//exit condition
								mouse_unsubscribe_int(hook_id);
								disable_data_reporting();	
								return 0;
							}
							bytecnt = 0;
							
							
						}
					}
                break;
				default:
					break; 
			}
		}
	}
	
	/*
	*  Unsubscribes mouse interrupts and disables data reporting 
	*/

	mouse_unsubscribe_int(hook_id);
	disable_data_reporting();
    return 0;
}
