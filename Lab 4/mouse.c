#include <minix/sysutil.h>
#include <lcom/lcf.h>
#include "mouse.h"
#include "macros.h"

extern uint32_t code;

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
	
int (mouse_subscribe_int)(uint8_t *bit_no) {
 
 int hook_id = 2;
 
 sys_irqsetpolicy(mouse_irq_line , IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id);
 
 *bit_no = hook_id;

  return *bit_no;
}

int (mouse_unsubscribe_int)(uint8_t *bit_no) {
	
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

int (enable_data_reporting) () {
	uint32_t *temp;
	temp = (uint32_t*) malloc (1);
	
	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (enableDataReporting);
	
	sys_inb(OUT_BUF, temp);

	if (*temp != ACK)
		return 1;
	
	return 0;
}

int (disable_data_reporting) ()
{
	uint32_t *temp;
	temp = (uint32_t*) malloc (1);
	
	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (disableDataReporting);
	
	sys_inb(OUT_BUF, temp);
	
	if (*temp != ACK)
		return 1;
	
	return 0;
	
}

int (set_data_reading)() {
	
	uint32_t *temp;
	temp = (uint32_t*) malloc (1);
	
	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (readData);
	
	sys_inb(OUT_BUF, temp);
	
	if (*temp != ACK)
		return 1;
	
	return 0;
	
}

int (set_stream_mode)()
{
	uint32_t *temp;
	temp = (uint32_t*) malloc (1);
	
	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (setStreamMode);
		
	sys_inb(OUT_BUF, temp);
	
	if (*temp != ACK)
		return 1;
	
	return 0;
}

int (reset_mouse) ()
{
	uint32_t *temp;
	temp = (uint32_t*) malloc (1);
	
	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (mouseReset);
	
	sys_inb(OUT_BUF, temp);
	
	if (*temp != ACK)
		return 1;
	
	return 0;
}

int (default_mouse) ()
{
	
	uint32_t *temp;
	temp = (uint32_t*) malloc (1);

	kbc_issue_cmd(writeToMouse);
	kbc_write_argument (setDefaults);	
	
	sys_inb(OUT_BUF, temp);
	
	if (*temp != ACK)
		return 1;
	
	return 0;
	
}

void (default_kbc)()
{
	uint8_t dflt = minix_get_dflt_kbc_cmd_byte();
	kbc_issue_cmd(kbc_write_command); 
	kbc_write_argument(dflt);
}

int (get_m_status) (){
	uint32_t sreturn;
	uint32_t bytes[3];
	struct m_status pstatus;
	
	if ( kbc_issue_cmd(writeToMouse) )
		return 1;
	
	if ( kbc_write_argument (requestStatus) )
		return 1;
	
	
	
	sys_inb(OUT_BUF, &sreturn);
	
	if(sreturn != ACK)
		return 1;
	
	bytes[0] = sreturn;
	kbc_read_return (&bytes[1]);
	kbc_read_return (&bytes[2]);
	pstatus = parse_m_status (bytes[0], bytes[1], bytes[2]);
	print_m_status (&pstatus);
	return 0;
	
}

struct m_status (parse_m_status) (uint8_t byte1, uint8_t byte2, uint8_t byte3){
	
	struct m_status pstatus;
	
	pstatus.bytes[0] = byte1;
	pstatus.bytes[1] = byte2;
	pstatus.bytes[2] = byte3;
	
	pstatus.fixed0 = (byte1 & fxd0);
	pstatus.mode = (byte1 & m_mode);
	pstatus.data_reporting = (byte1 & dreport);
	pstatus.scaling = (byte1 & sclng);
	pstatus.fixed00 = (byte1 & fxd00);
	pstatus.lb = (byte1 & left_b);
	pstatus.mb = (byte1 & middle_b);
	pstatus.rb = (byte1 & right_b);
	
	return pstatus;	
}

void (print_m_status) (struct m_status *ms){
	
	printf("B1 = %X  ", ms->bytes[0]);
	printf("B2 = %X  ", ms->bytes[1]);
	printf("B3 = %X  ", ms->bytes[2]);
	printf("Always 0 = %X  ", ms->fixed0);
	if (ms->mode)
		printf("Mode = remote\n");
	else printf ("Mode = streaming\n");
	
	if (ms->data_reporting)
		printf ("Data Reporting = enabled   ");
	else printf ("Data Reporting = disabled   ");
	
	if (ms->scaling)
		printf ("Scaling = 2:1   ");
	else printf ("Scaling = 1:1   ");
	printf("Always 0 = %X\n", ms->fixed00);
	printf("LB = %X   ", ms->lb);
	printf("MB = %X   ", ms->mb);
	printf("RB = %X\n", ms->rb);
	
}

uint32_t (kbc_read_command)(){
	kbc_issue_cmd(0x20);
	
	uint32_t cmdWord;
	sys_inb(OUT_BUF, &cmdWord);
	
	return cmdWord;
}

int (state_machine) (struct mouse_ev *ms, state_t *ges, uint8_t x_len, uint8_t tolerance){
	
	static uint16_t x_sum;
	static uint16_t y_sum;

	switch (*ges){
		case 0: //INITIAL STATE
			if (ms->type == LB_PRESSED)
				*ges = DRAWLINE1;
			break;
		case 1: //DRAWING FIRST LINE
			if (ms->type == LB_PRESSED){
				if ( (ms->delta_x & negative_16) && ( (ms->delta_x & abs_16) > tolerance) ){
					*ges = INIT;
				}
				else if ( (ms->delta_y & negative_16) && ( (ms->delta_y & abs_16) > tolerance) ){
						*ges = INIT;
				}
				else { 
					x_sum += ms->delta_x;  
					y_sum += ms->delta_y;
				}
			}
			else if (ms->type == LB_RELEASED){
				double slope = (double)y_sum / x_sum;
				if ( ( slope >= 1.0) && (x_sum >= x_len) ){
					*ges = VERTEX;
					x_sum = 0;
					y_sum = 0;
				}
				else *ges = INIT;
			}
			
			break;
		case 2: //SUCESSFULLY REACHED VERTEX
			x_sum += ms->delta_x;
			y_sum += ms->delta_x;
			
			if ( (x_sum > tolerance) || (y_sum > tolerance) || ( ms->type != MOUSE_MOV && ms->type != RB_PRESSED ) )
				*ges = INIT;
			else if (ms->type == RB_PRESSED){
				x_sum = 0;
				y_sum = 0;
				*ges = DRAWLINE2;
			}
			break;
		case 3://DRAWING SECOND LINE
			if (ms->type == RB_PRESSED){
				if ( (ms->delta_x & negative_16) && ( (ms->delta_x & abs_16) > tolerance) ){
					*ges = INIT;
				}
				else if ( !(ms->delta_y & negative_16) && ( ms->delta_y > tolerance) ){
						*ges = INIT;
				}
				else { 
					x_sum += ms->delta_x;  
					y_sum += (ms->delta_y & abs_16);
				}
			}
			else if (ms->type == RB_RELEASED){
				double slope = (double)y_sum / x_sum;
				if ( ( slope >= 1.0) && (x_sum >= x_len) ){
					*ges = COMP;
					x_sum = 0;
					y_sum = 0;
				}
				else *ges = INIT;
			}
			
			break;
		case 4://COMPLETED
			break;
	}
return 0;	
}


struct mouse_ev (m_detect_evt) (struct packet *pp){
	
	static struct mouse_ev previous_ev;
	
	
	struct mouse_ev current_ev;
    current_ev.type = MOUSE_MOV; //MOUSE MOVEMENT BY DEFAULT
	
	
	//LB AND RB PRESSED, or MB PRESSED 	
	if ( ( pp->lb && pp->rb ) || pp->mb ){
		current_ev.type = BUTTON_EV;
		previous_ev = current_ev;
		return current_ev;
	}
	
	//LB RELEASED
	if ( ( previous_ev.type == LB_PRESSED ) && !(pp->lb) ){
		current_ev.type = LB_RELEASED;
		previous_ev = current_ev;
		return current_ev;
	}
	
	//RB RELEASED
	if ( ( previous_ev.type == RB_PRESSED ) && !(pp->rb) ){
		current_ev.type = RB_RELEASED;
		previous_ev = current_ev;
		return current_ev;
	}
	
	//LB PRESSED
	if (pp->lb){
		current_ev.type = LB_PRESSED;
		current_ev.delta_x = pp->delta_x;
		current_ev.delta_y = pp->delta_y;
		previous_ev = current_ev;
		return current_ev;
	}
	
	//RB PRESSED
	if (pp->rb){
		current_ev.type = RB_PRESSED;
		previous_ev = current_ev;
		current_ev.delta_x = pp->delta_x;
		current_ev.delta_y = pp->delta_y;
		return current_ev;
	}
	current_ev.delta_x = pp->delta_x;
	current_ev.delta_y = pp->delta_y;
	
return current_ev;
	
}
