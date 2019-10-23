#include "multiplayer.h"
#include "vcard.h"
#include "serialport.h"
#include "devices.h"
#include <stdio.h>
#include <stdint.h>

void serial_interrupt_handler (uint16_t COM_ADDR, game_data *game){
	
	uint8_t serialdata;
	sp_read_buffer(COM_ADDR, &serialdata);
	//printf("serial = %c\n", serialdata); //DEGUB
	static unsigned int step = 0;
	
	switch (game->serial_event){
		
		case NEWTRNM: {
			if (!game->mpconnection && serialdata == 'O')
				step++;
			if (!game->mpconnection && serialdata == 'L' && step){
				game->serial_event = C_MSG;
				game->mpconnection = true;
				step = 0;
				break;
			}
			if (game->mpconnection && serialdata == 'K')
				game->serial_event = KB;
			else if (game->mpconnection && serialdata == 'M')
				game->serial_event = MS;
			else if (game->mpconnection && serialdata == 'S')
				game->serial_event = SC;
			else if (game->mpconnection && serialdata == 'H' && !game->host)
				game->serial_event = HP;
			break;
		}
		
		case KB:{
				game->p2_keyboard_event = serialdata;
				mp_ship_apply_force(&game->p2_keyboard_event, &game->player2);
				game->serial_event = NEWTRNM;
			
			break;
		}
		
		case MS:{
			static unsigned int ms_t_step = 0;
			static uint8_t bytes[3];
			if (ms_t_step == 0){
				bytes[0] = serialdata;
				ms_t_step++;
				break;
			}
			else if (ms_t_step == 1){
				bytes[1] = serialdata;
				ms_t_step++;
				break;
			}
			else if (ms_t_step == 2){
				bytes[2] = serialdata;
				ms_t_step = 0;
				game->p2_mouse_event = parse_mpacket(bytes[0], bytes[1], bytes[2]);
				mp_crossh_update(&game->p2_mouse_event, &game->player2);
				
				if (game->p2_mouse_event.lb && game->player2.weapon_ready)
					ship_fire_laser(&game->player2, &game->timers.player2_weapon_timer);
				
				game->serial_event = NEWTRNM;
				break;
			}
			break;
		}
		case SC:{
			
			static int sc_t_step = 0;
			static int16_t pivot_x, pivot_y;
                        
			if (sc_t_step == 0){
					pivot_x = serialdata << 8;                                
					sc_t_step++;
					break;
			}
			else if (sc_t_step == 1){
					pivot_x += serialdata;                                
					sc_t_step++;
					break;
			}
			else if (sc_t_step == 2){
					pivot_y = serialdata << 8;                                
					sc_t_step++;
					break;
			}
			else if (sc_t_step == 3){
					pivot_y += serialdata;                                
					sc_t_step = 0;
					game->player2.pivot.x = pivot_x;
					game->player2.pivot.y = pivot_y;
					game->serial_event = NEWTRNM;
					break;
			}
			
			
			break;
		}
		case HP:{
			static unsigned int hp_t_step = 0;
			if (hp_t_step == 0){
				game->player2.hp = serialdata;
				hp_t_step++;
				break;
			}
			else if (hp_t_step == 1){
				game->player1.hp = serialdata;
				hp_t_step = 0;
				game->serial_event = NEWTRNM;
				break;
			}
			break;
		}
		
		default: break;
	}
}

void mp_connect (game_data *game){
	
	uint16_t comaddr;
	if (game->host)
		comaddr = COM2_ADDR;
	else comaddr = COM1_ADDR;
	
	if (game->timers.timerTick % 30 == 0 && !game->host)
		mp_transmit_connect (comaddr);
	
	if (game->event == SERIALPORT){
		if (game->serial_event == C_MSG){
			if (game->host)
				mp_transmit_connect (comaddr);
			game->state = MULTIPLAYER;
			game->serial_event = NEWTRNM;
			mp_ship_spawn(game);
			drawBitmap(game->bmp.connected, 0, 0);
			display_frame(game->settings.page_flip, game->settings.vsync);
			
			return;
		}
	}
}

void mp_ship_spawn(game_data *game){

	game->player1.cannon.x = -300;
	game->player1.cannon.y = 0;
	game->player2.cannon.x = 300;
	game->player2.cannon.y = 0;
	
	game->player1.pivot.x = -280;
	game->player1.pivot.y = 0;
	game->player2.pivot.x = 280;
	game->player2.pivot.y = 0;
	
	game->player1.port.x = 0;
	game->player1.port.y = -1;
	game->player2.port.x = 0;
	game->player2.port.y = 1;
	
	game->player1.starboard.x = 0;
	game->player1.starboard.y = 1; 
	game->player2.starboard.x = 0;
	game->player2.starboard.y = -1; 
	
	game->player1.force.x = 0;
	game->player1.force.y = 0;
	game->player2.force.x = 0;
	game->player2.force.y = 0;
	
	game->player1.crosshair.x = -230;
	game->player1.crosshair.y = 0;
	game->player2.crosshair.x = 230;
	game->player2.crosshair.y = 0;
	
	game->player1.hp = PLAYER_MAX_HEALTH;
	game->player1.hit_radius = SHIP_HITRADIUS;
	game->player2.hp = PLAYER_MAX_HEALTH;
	game->player2.hit_radius = SHIP_HITRADIUS;

	game->keyboard_event = IDLING;
	game->p2_keyboard_event = IDLING;
		
	for (unsigned int i = 0; i < AMMO; i++){
		game->player1.lasers[i].active = false;
		game->player2.lasers[i].active = false;
	}
	
	if (!game->host){
		player temp = game->player1;
		game->player1 = game->player2;
		game->player2 = temp;
	}
}

void mp_ship_apply_force (kb_evt_tp *keyboard_event, player *p){
	
	/* Creates necessary versors */
	mvector vcannon = mvector_create (p->pivot, p->cannon);
	mvector cannon_versor = mvector_get_versor(&vcannon);

	
	/* Applies force based on keyboard input and accelaration values in "macros.h" */
	switch (*keyboard_event) {
		case MAIN_THRUSTER:{
			mvector_multiplication(&cannon_versor, 5);
			mvector_add (&p->force, &cannon_versor);
			break;
		}
		case REVERSE_THRUSTER:{
			mvector_multiplication(&cannon_versor, 5);
			mvector_subtract (&p->force, &cannon_versor);
			break;
		}
		default: break;	
	}
	
	/* Limits the force vector, and effectively the ship's velocity, to the maximum velocity value in "macros.h" */
	mvector_limit(&p->force, THRUSTERS_MAXIMUM_VELOCITY);
}

void mp_crossh_update(struct packet *mouse_event, player *p){
	
	/* Updates the crosshair's position */					
	p->crosshair.x += mouse_event->delta_x;
	p->crosshair.y += mouse_event->delta_y;
	
	/* Verifies the crosshair's bounds */	
	if (p->crosshair.x > math_h_positive_bound - 5)
		p->crosshair.x = math_h_positive_bound - 5;
	if (p->crosshair.x < math_h_negative_bound + 5)
		p->crosshair.x = math_h_negative_bound + 5;
		
	if (p->crosshair.y > math_v_positive_bound - 5)
		p->crosshair.y = math_v_positive_bound - 5;
	if (p->crosshair.y < math_v_negative_bound + 5)
		p->crosshair.y = math_v_negative_bound + 5;	
}

bool mp_ship_collision (game_data *game){
	bool hp_change = false;
	for (unsigned int i = 0; i < AMMO; i++){
		
		if (game->player1.lasers[i].active){
			mvector v_ast_laser = mvector_create(game->player1.lasers[i].position, game->player2.pivot);
			double distance = mvector_magnitude(&v_ast_laser);
			if (distance <= game->player2.hit_radius){
				game->player1.lasers[i].active = false;
				if (game->host){
					game->player2.hp -= 15;
					if (game->player2.hp < 0)
						game->player2.hp = 0;
					hp_change = true;
				}
			}
		}

		if (game->player2.lasers[i].active){
			mvector v_ast_laser2 = mvector_create(game->player2.lasers[i].position, game->player1.pivot);
			double distance2 = mvector_magnitude(&v_ast_laser2);
			if (distance2 <= game->player1.hit_radius){
				game->player2.lasers[i].active = false;
				if (game->host){
					game->player1.hp -= 15;
					if (game->player1.hp < 0)
						game->player1.hp = 0;
					hp_change = true;
				}			
			}
		}
	}
	
	return hp_change;
}

void mp_transmit_keyboard_ev(uint16_t COM_ADDR, game_data *game){
	uint32_t lsr;
	unsigned int kb_t_step = 0;
	while (true){
		
		sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
		while (! (lsr & SP_TX_READY) )
			sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
	
		if (kb_t_step == 0){
			sp_write_th (COM_ADDR, 'K');
			kb_t_step++;
		}
		else if (kb_t_step == 1) {
			sp_write_th(COM_ADDR, (uint32_t)game->keyboard_event);
			kb_t_step = 0;
			break;
		}
	}
	
}

void mp_transmit_mouse_ev(uint16_t COM_ADDR, game_data *game){
	
	uint32_t lsr;
	unsigned int ms_t_step = 0;
	while (true){
		sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
		while (! (lsr & SP_TX_READY) )
			sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
			
		if (ms_t_step == 0){
			sp_write_th (COM_ADDR, 'M');
			ms_t_step++;
		}
		else if (ms_t_step == 1) {
			sp_write_th(COM_ADDR, game->mouse_event.bytes[0]);
			ms_t_step++;
		}
		else if (ms_t_step == 2) {
			sp_write_th(COM_ADDR, game->mouse_event.bytes[1]);
			ms_t_step++;
		}
		else if (ms_t_step == 3) {
			sp_write_th(COM_ADDR, game->mouse_event.bytes[2]);
			ms_t_step = 0;
			break;
		}
	}
}

void mp_transmit_hp(uint16_t COM_ADDR, game_data *game){
	
	uint32_t lsr;
	unsigned int hp_t_step = 0;
	while (true){
		sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
		while (! (lsr & SP_TX_READY) )
			sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
			
		if (hp_t_step == 0){
			sp_write_th (COM_ADDR, 'H');
			hp_t_step++;
		}
		else if (hp_t_step == 1) {
			sp_write_th(COM_ADDR, (uint32_t)game->player1.hp);
			hp_t_step++;
		}
		else if (hp_t_step == 2) {
			sp_write_th(COM_ADDR, (uint32_t)game->player2.hp);
			hp_t_step = 0;
			break;
		}
	}
}


void mp_transmit_connect (uint16_t COM_ADDR){
	
	unsigned int msg_step = 0;
	uint32_t lsr;
	while (true){
		sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
		
		while (! (lsr & SP_TX_READY) ){
			sys_inb( (COM_ADDR + SP_LINE_STATUS) , &lsr);
		}
		if (msg_step == 0){
			sp_write_th (COM_ADDR, 'O');
			msg_step++;
		}
		else if (msg_step == 1){
			sp_write_th (COM_ADDR, 'L');
			break;
		}	
	}
}



void mp_playing_event_handler(game_data* game){
	if (game->player1.hp <= 0 || game->player1.hp > 100 ||  game->player2.hp <= 0 || game->player2.hp > 100){
		game->state = MULTIPLAYEROVER;
		return;
	}
	
	uint16_t commaddr;
	if (game->host)
		commaddr = COM2_ADDR;
	else commaddr = COM1_ADDR;
	
	switch (game->event){
		case KEYBOARD:{
			mp_transmit_keyboard_ev(commaddr, game);
			mp_ship_apply_force(&game->keyboard_event, &game->player1);
			if (game->keyboard_event == QUIT)
				game->state = COMP;
			break;
		}
	
		case MOUSE:{
			mp_transmit_mouse_ev(commaddr, game);
			mp_crossh_update(&game->mouse_event, &game->player1);
						
			if (game->mouse_event.lb && game->player1.weapon_ready)
				ship_fire_laser(&game->player1, &game->timers.player1_weapon_timer);
			
			break;
		}
			
		case TIMER:{
			
			/* Physics update */
			if (game->timers.timerTick % PHYSICS_TICKS == 0){
				mp_physics_update(game);
				if (mp_ship_collision(game) && game->host)
					mp_transmit_hp(commaddr, game);
			}
							
			/* Locked fps render */
			if (game->settings.fps){
				if (game->timers.timerTick % game->settings.fps == 0)
					mp_handle_frame(game);
			}
									
			/* Fire rate controller */
			if (game->timers.player1_weapon_timer >= (60 / FIRE_RATE) && !(game->player1.weapon_ready) )
				game->player1.weapon_ready = true;
			
			if (game->timers.player2_weapon_timer >= (60 / FIRE_RATE) && !(game->player2.weapon_ready) )
				game->player2.weapon_ready = true;
			
			break;
		}
		case RTC: {
			game->timers.frames_per_second = game->timers.framecounter;
			game->timers.framecounter = 0;
			break;
		}
		default: break;
	}
	

	
}
