#include <minix/sysutil.h>
#include <lcom/timer.h>
#include <minix/syslib.h>
#include <machine/asm.h>
#include <stdint.h>

#include "macros.h"
#include "devices.h"
#include "serialport.h"


static int timer_hook_id = 1;
static int mouse_hook_id = 2;
static int keyboard_hook_id = 3;
static int rtc_hook_id = 4;
uint32_t code = 0;

/* Enable/Disable mouse interrupts and data reporting*/

int (enable_data_reporting) () {

	uint32_t temp;

	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (enableDataReporting);
	
	__asm (
		"movl $0, %%eax \n"
		"INb $0x60, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(temp)                         
		: "g"(temp)
		: "%eax"   
	);

	if (temp != ACK)
		return 1;
	
	return 0;
}

void mouse_init () {
 
	enable_data_reporting();
 	sys_irqsetpolicy(mouse_irq_line , IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id);
}

int (disable_data_reporting) ()
{
	uint32_t temp;
	
	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (disableDataReporting);
	
	__asm (
		"movl $0, %%eax \n"
		"INb $0x60, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(temp)                         
		: "g"(temp)
		: "%eax"   
	);
	
	if (temp != ACK)
		return 1;
	
	return 0;
}
	
void mouse_stop () {
	
	sys_irqrmpolicy (&mouse_hook_id);
	disable_data_reporting();
}

/* Enable/Disable keyboard interrupts */

void keyboard_subscribe () {
	
 sys_irqsetpolicy(kb_irq_line, IRQ_REENABLE | IRQ_EXCLUSIVE, &keyboard_hook_id);
}

void keyboard_unsubscribe () {
	
	sys_irqrmpolicy (&keyboard_hook_id);
}

/* Enable/Disable timer 0 interrupts*/

void timer_subscribe () {
 
	sys_irqsetpolicy(timer_irq_line, IRQ_REENABLE, &timer_hook_id);
}

void timer_unsubscribe () {
	
	sys_irqrmpolicy (&timer_hook_id);
}

/* Enable/Disable RTC interrupts*/

void rtc_subscribe() {
	
	sys_irqsetpolicy(rtc_irq_line, IRQ_REENABLE, &rtc_hook_id);
}

void rtc_unsubscribe() {
	
	sys_irqrmpolicy(&rtc_hook_id);
}

void rtc_enable_int(){
	uint8_t reg_b_cfg = readRTC(REGISTER_B);
	reg_b_cfg |= BIT(4);
	writeRTC(REGISTER_B, reg_b_cfg);
}

void rtc_disable_int(){
	uint8_t reg_b_cfg = readRTC(REGISTER_B);
	reg_b_cfg &= ~BIT(4);
	writeRTC(REGISTER_B, reg_b_cfg);
}

void init_devices(){
	keyboard_subscribe();
	mouse_init();
	timer_subscribe();
	rtc_subscribe();
	rtc_enable_int();
	sp_subscribe_int();
	sp_enable_int();
	sp_set_rate(COM1_ADDR, 57600);
	sp_set_rate(COM2_ADDR, 57600);
}

void reset_devices(){
	keyboard_unsubscribe();
	mouse_stop();
	timer_unsubscribe();
	rtc_disable_int();
	rtc_unsubscribe();
	sp_disable_int();
	sp_unsubscribe_int();
}

/* INTERRUPT HANDLERS */

void timer_interrupt_handler(game_timers *timers) {
	timers->timerTick++;
	timers->player1_weapon_timer++;
	timers->player2_weapon_timer++;
	timers->alien_weapon_timer++;
	timers->teleport_timer++;
	timers->round_timer++;
	if (timers->alien_death_timer > 0)
		timers->alien_death_timer--;
}

struct packet (parse_mpacket) (uint8_t byte1, uint8_t byte2, uint8_t byte3){//Parses mouse packet
		
		uint16_t x_delta = byte2,  y_delta = byte3;
		struct packet mouseP;
		
		/*
		*    BYTE ARRAY
		*/
		
		mouseP.bytes[ 0 ] = byte1;
		mouseP.bytes[ 1 ] = byte2;
		mouseP.bytes[ 2 ] = byte3;
		
		/*
		*    OVERFLOWS
		*/
		
		mouseP.y_ov = ( byte1 & y_ovf );
		mouseP.x_ov = ( byte1 & x_ovf );
		
		/*
		*    y Delta
		*/
		
		if ( byte1 & y_sign )
			y_delta |= 0xFF00;
		else y_delta &= 0x00FF;
		mouseP.delta_y = y_delta;
		
		
		/*
		*    xDelta
		*/
		
		if ( byte1 & x_sign )
			x_delta |= 0xFF00;
		else x_delta &= 0x00FF;
		mouseP.delta_x = x_delta;
		
		/*
		*    MOUSE BUTTONS
		*/
		
		mouseP.mb = ( byte1 & mouse_mb );
		mouseP.rb = ( byte1 & mouse_rb );
		mouseP.lb = ( byte1 & mouse_lb );		
		return mouseP;
			
}

int mouse_interrupt_handler(struct packet *ms_ev){
	
	static unsigned int bytecnt = 0;//Keeps track of the packet's byte number
	static uint8_t bytes[3];//Stores the packet
	uint32_t stat;
	uint32_t data;
	
	__asm (
		"movl $0, %%eax \n"
		"INb $0x64, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(stat)                         
		: "g"(stat)
		: "%eax"   
	);
	
	if (stat & OBF)
		__asm (
			"movl $0, %%eax \n"
			"INb $0x60, %%al \n"
			"movl %%eax, %0 \n"
			:  "=g"(data)                         
			: "g"(data)
			: "%eax"   
		);
	else return 0;
	if ((stat & (PAR_ERR | TO_ERR ) ) == 0)
		code = data;
	else return 0;
	
	/*
	* Reads byte 1
	*/
						
	if (bytecnt == 0){
		if (code & byte1_fixed){
			bytes[0] = code;
			bytecnt++;
		}							
		return 0;
	}
						
	/*
	* Reads byte 2
	*/
						
	if (bytecnt == 1){
		bytes[1] = code;
		bytecnt++;
		return 0;
	}
						
	/*
	* Reads byte 3, parses and prints packet, resets packet cycle
	*/
						
	if (bytecnt == 2){
		bytes[2] = code;
		*ms_ev = parse_mpacket(bytes[0], bytes[1], bytes[2]);
		bytecnt = 0;
		return 1;
	}
	return 0;
}

void keyboard_interrupt_handler (kb_evt_tp *evt){
	
	__asm (
		"movl $0, %%eax \n"
		"INb $0x60, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(code)                         
		: "g"(code)
		: "%eax"   
	);
	
	switch(code){
		case 0x11:	*evt = MAIN_THRUSTER;	break;
		case 0x1e:	*evt = PORT_THRUSTER;	break;
		case 0x20:	*evt = STARBOARD_THRUSTER;	break;
		case 0x1f:	*evt = REVERSE_THRUSTER;	break;
		case 0x81:	*evt = K_ESC;	break;
		case 0x39:	*evt = QUIT; break;
			
		default: *evt = IDLING;
	}
}




/* KBC AUXILIARY FUNCTIONS */

int (kbc_issue_cmd)(uint8_t cmd) {
	uint32_t stat;
	
	unsigned int i;

	__asm (
		 "movl $0, %%eax \n"
		 "INb $0x64, %%al \n"
         "movl %%eax, %0 \n"
         :  "=g"(stat)                         
         : "g"(stat)
         : "%eax"   
	);
	
	for (i = 0; i <= 100; i++){
		
		if ( (stat & IBF) != 0 ) {		
			__asm (
				"movl $0, %%eax \n"
				"INb $0x64, %%al \n"
				"movl %%eax, %0 \n"
				:  "=g"(stat)                         
				: "g"(stat)
				: "%eax"   
			);
		}
		else break;
		
		tickdelay(micros_to_ticks(DELAY_US));
	}
	
	if (i == 100)
		return 1;
	
	__asm (
		"movl %0, %%eax \n"
		"OUTb %%al, $0x64 \n"
		:  "=g"(cmd)                         
		: "g"(cmd)
		: "%eax"   
	);
	
	return 0;
}

int (kbc_write_argument)(uint32_t arg){
	
	uint32_t stat;
	unsigned int i;
	
	__asm (
		 "movl $0, %%eax \n"
		 "INb $0x64, %%al \n"
         "movl %%eax, %0 \n"
         :  "=g"(stat)                         
         : "g"(stat)
         : "%eax"   
	);
	
	for (i = 0; i <= 100; i++){
	
		if ( (stat & OBF) != 0 ) {	
			__asm (
				"movl $0, %%eax \n"
				"INb $0x64, %%al \n"
				"movl %%eax, %0 \n"
				:  "=g"(stat)                         
				: "g"(stat)
				: "%eax"   
			);
		}
		else break;
		
		tickdelay(micros_to_ticks(DELAY_US));
	}
	
	if (i == 100)
		return 1;
	
	__asm (
		"movl %0, %%eax \n"
		"OUTb %%al, $0x60 \n"
		:  "=g"(arg)                         
		: "g"(arg)
		: "%eax"   
	);
	
	return 0;
}

int (kbc_read_return) (uint32_t *arg){
	
	unsigned int i;
	uint32_t stat;
	
	__asm (
		"movl $0, %%eax \n"
		"INb $0x64, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(stat)                         
		: "g"(stat)
		: "%eax"   
	);

	
	for (i = 0; i <= 100; i++) {
		
		if ( (stat & OBF) == 0 ) {
			__asm (
				"movl $0, %%eax \n"
				"INb $0x64, %%al \n"
				"movl %%eax, %0 \n"
				:  "=g"(stat)                         
				: "g"(stat)
			: "%eax"   
			);
		}
		else break;
		tickdelay(micros_to_ticks(DELAY_US));
	}
	
		__asm (
		"movl $0, %%eax \n"
		"INb $0x60, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(*arg)                         
		: "g"(*arg)
		: "%eax"   
	);
	
	if ( (stat & (PAR_ERR | TO_ERR) ) == 0 && i < 100)
		 return 0;
	else return 1;
	
}

int (verify_stat_error)(){

	uint32_t stat;
	
	__asm (
		"movl $0, %%eax \n"
		"INb $0x64, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(stat)                         
		: "g"(stat)
		: "%eax"   
	);

	unsigned int c = 0;
	while(c <= 100 && ( stat & (PAR_ERR | TO_ERR) ) != 0 && ( stat & 0x20) != 0)
	{
			tickdelay(micros_to_ticks(DELAY_US));
			c++;
	}
	
	if(c==100)
		return 1;
	
	return 0;
	
}


/* RTC AUXILIARY FUNCTIONS */

uint8_t readBCD(uint8_t bcd) {
	bcd = ( ( (bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F) );
	return bcd;
}

uint8_t readRTC(uint32_t ADDR){
	uint32_t value;
	
	__asm (
		"movl %0, %%eax \n"
		"OUTb %%al, $0x70 \n"
		:  "=g"(ADDR)                     
		: "g"(ADDR)
		: "%eax"   
	);
		
	__asm (
		"movl $0, %%eax \n"
		"INb $0x71, %%al \n"
		"movl %%eax, %0 \n"
		:  "=g"(value)                         
		: "g"(value)
		: "%eax"   
	);	

	return (uint8_t) value;
}

void writeRTC(uint32_t ADDR, uint32_t data){
	__asm (
		"movl %0, %%eax \n"
		"OUTb %%al, $0x70 \n"
		:  "=g"(ADDR)                     
		: "g"(ADDR)
		: "%eax"   
	);
		
	__asm (
		"movl %0, %%eax \n"
		"OUTb %%al, $0x71 \n"
		:  "=g"(data)                     
		: "g"(data)
		: "%eax"   
	);
}

void readTime(rtc_time_t *time) {
	
	bool bcd = true;
	uint8_t reg_b_cfg = readRTC(REGISTER_B);
	uint8_t temp = reg_b_cfg;
	
	if (reg_b_cfg & BIT(2))
		bcd = false;
	
	reg_b_cfg |= BIT(7);
	writeRTC(REGISTER_B, reg_b_cfg);
	
	time->hours = readRTC(RTC_HRS);
	time->minutes = readRTC(RTC_MIN);
	time->seconds = readRTC(RTC_SEC);
	
	if (bcd){
	time->hours = readBCD(time->hours);
	time->minutes = readBCD(time->minutes);
	time->seconds = readBCD(time->seconds);
	}
	
	temp &= ~BIT(7);
	writeRTC(REGISTER_B, temp);

}
