#include "gamestate.h"
#include "vcard.h"
#include "multiplayer.h"

void cursor_update (game_data *game){
	
	if (! (game->cursor.x + game->mouse_event.delta_x > hres - 10 || game->cursor.x + game->mouse_event.delta_x < 10) )
		game->cursor.x += game->mouse_event.delta_x * game->settings.m_sens;
	
	if (! (game->cursor.y - game->mouse_event.delta_y > vres - 10 || game->cursor.y - game->mouse_event.delta_y < 10) )
		game->cursor.y -= game->mouse_event.delta_y * game->settings.m_sens;
}

void physics_update (game_data *game){
	
	ast_update(game->asteroid_field);
	ast_collision(game->asteroid_field, &game->player1, &game->alien);				
	ship_update(&game->player1);
	if (game->alien.active){
		alien_update(&game->alien, &game->player1, &game->timers);
		alien_collision(&game->alien, &game->player1, &game->timers);
	}
	if (game->timers.round_timer <= 30)
		game->player1.invulnerability = true;
	else game->player1.invulnerability = false;
	
	if (game->player1.hp <= 0){
		game->player1.hp = 0;
		game->state = LOSS;
	}
}

void mp_physics_update (game_data *game){
		
	ship_update(&game->player1);
	ship_update(&game->player2);
	if (game->player1.hp <= 0)
		game->state = COMP;
}

void handle_frame (game_data * game){
	
	render_frame(game);
	display_frame(game->settings.page_flip, game->settings.vsync);
	game->timers.framecounter += 1;
}

void mp_handle_frame (game_data * game){
	
	mp_render_frame(game);
	display_frame(game->settings.page_flip, game->settings.vsync);
	game->timers.framecounter += 1;
}

void handle_menu_frame (game_data *game, Bitmap *bckgrd) {
	
	drawBitmap(bckgrd, 0, 0);

	switch (game->state){
		case MENU:{
			for (int i = 0; i < 5; i++)
				draw_number(game->highscores[i], 900, 60 + i * 40 , game);
			draw_time_of_day(game, 10, 10);
			break;
		}
		case OPTIONSMENU: {
			if (game->settings.fps_counter)
				drawBitmap (game->bmp.boxticked, 369, 173);
			
			if (!game->settings.fps)
				drawBitmap (game->bmp.boxticked, 870, 74);
			else if (game->settings.fps == 1)
				drawBitmap (game->bmp.boxticked, 661, 75);
			else drawBitmap (game->bmp.boxticked, 548, 77);
			
			if (game->settings.m_sens < 1)
				drawBitmap (game->bmp.boxticked, 583, 99);
			else if (game->settings.m_sens == 1)
				drawBitmap (game->bmp.boxticked, 712, 100);
			else drawBitmap (game->bmp.boxticked, 838, 100);
			
			if (game->settings.page_flip)
				drawBitmap (game->bmp.boxticked, 678, 127);
			if (game->settings.vsync){
				drawBitmap (game->bmp.boxticked, 678, 127);
				drawBitmap (game->bmp.boxticked, 575, 150);
			}
			
			break;
		}
		default: break;
	
}
	
	draw_pixmap(&game->xpm.cursor, game->cursor.x + 5 , game->cursor.y + 7, false);
	
	display_frame(game->settings.page_flip, game->settings.vsync);
}

void start_timers (game_timers *timers){
	
	timers->framecounter = 1;
	timers->frames_per_second = 0;
	timers->cyclecounter = 0;
	timers->player1_weapon_timer = 0;
	timers->player2_weapon_timer = 0;
	timers->teleport_timer = 0;
	timers->round_timer = 0;
	timers->alien_death_timer = 0;
}

void game_data_init(game_data *game){

	if (!load_highscores(game->FILEPATH, game->highscores))
		for (int i = 0; i < 5; i++)
			game->highscores[i] = 0;
		
	game->time_of_day.seconds = 0;
	game->time_of_day.minutes = 0;
	game->time_of_day.hours = 0;
	game->timers.timerTick = 0;
	game->timers.start_seq = 3;
	game->state = MENU;
	game->serial_event = NEWTRNM;
	game->timers.cyclecounter = 0;
	game->cursor.x = hres/2;
	game->cursor.y = vres/2;
	
	game->mpconnection = false;
	game->settings.fps_counter = true;
	game->settings.fps = 1;
	game->settings.page_flip = false;
	game->settings.vsync = false;
	game->settings.m_sens = 1;
	game->alien.active = false;
	
	load_xpms(game);
	load_bitmaps(game);
}

void interrupt_handler(game_data* game){
	
	//Driver receive variables
	int ipc_status;
	message msg;
	
	/*
	*  Driver receive cycle, exits when state machine is in COMP state
	*/
	while (game->state != COMP) {
				
		game->timers.cyclecounter++;
		
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		
		if (is_ipc_notify(ipc_status)) {		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:{ 	
						
					/* MOUSE INTERRUPT */
					if ( msg.m_notify.interrupts & irq_mouse ) {
						if ( mouse_interrupt_handler(&game->mouse_event) ){
								game->event = MOUSE;
								game_state_machine(game);
						}
					}
					/* KEYBOARD INTERRUPT */
					if ( msg.m_notify.interrupts & irq_keyboard ) {
						keyboard_interrupt_handler(&game->keyboard_event);
						game->event = KEYBOARD;
						game_state_machine(game);
					}	
					
					/* TIMER INTERRUPT */
					if ( msg.m_notify.interrupts & irq_timer0 ){ 
						
						game->event = TIMER;
						timer_interrupt_handler(&game->timers); 		
						game_state_machine(game);
					}
					/* RTC INTERRUPT */
					if ( msg.m_notify.interrupts & irq_rtc ){ 
						game->event = RTC;
						game_state_machine(game);
						readRTC(REGISTER_C);
					}
					/* SERIAL PORT COM1 INTERRUPT */
					if ( msg.m_notify.interrupts & irq_com1_serial ) {
						if (sp_data_available(COM1_ADDR) && game->host){
							serial_interrupt_handler(COM1_ADDR, game);
							game->event = SERIALPORT;
							game_state_machine(game);
						}
					}
					/* SERIAL PORT COM2 INTERRUPT */
					if ( msg.m_notify.interrupts & irq_com2_serial ) {
						if (sp_data_available(COM2_ADDR) && !game->host){
							serial_interrupt_handler(COM2_ADDR, game);
							game->event = SERIALPORT;
							game_state_machine(game);
						}
					}		
							
			 		break;
				}
						
				default: break; 
			}
		}
		//UNLOCKED FPS RENDER
		if (!game->settings.fps && (game->timers.cyclecounter % 4 != 0)  && game->state == PLAYING) //A fourth of the frames are being dropped to ensure system stability
			handle_frame(game);
	}

}

void game_state_machine (game_data* game){
	
	switch (game->state){
			
		case MENU:{
			static bool first_frame = false;
			if (!first_frame){
				first_frame = true;
				handle_menu_frame(game, game->bmp.menu);
			}

			if (game->event == MOUSE){
				
				cursor_update(game);			
				//checking button clicks
				if (game->mouse_event.lb){
					if (game->cursor.x >= 229 && game->cursor.x <= 648 && game->cursor.y >= 132 && game->cursor.y <= 217) {
						game->state = START_SEQUENCE;
						break;
					}
					else if (game->cursor.x >= 229 && game->cursor.x <= 435 && game->cursor.y >= 413 && game->cursor.y <= 502) {
						game->state = COMP;
						break;
					}
					else if (game->cursor.x >= 229 && game->cursor.x <= 648 && game->cursor.y >= 223 && game->cursor.y <= 311){
						game->host = false;
						game->state = CONNECTING;
						drawBitmap(game->bmp.client_connecting, 0, 0);
						display_frame(game->settings.page_flip, game->settings.vsync);
						break;
					}
					else if (game->cursor.x >= 229 && game->cursor.x <= 648 && game->cursor.y >= 319 && game->cursor.y <= 408){
						game->host = true;
						game->state = CONNECTING;
						drawBitmap(game->bmp.host_connecting, 0, 0);
						display_frame(game->settings.page_flip, game->settings.vsync);
						break;
					}
					else if (game->cursor.x >= 448 && game->cursor.x <= 648 && game->cursor.y >= 413 && game->cursor.y <= 502){
						game->state = OPTIONSMENU;
						break;
					}
					
				}
				handle_menu_frame(game, game->bmp.menu);
			}
			if (game->event == RTC){
				
				readTime(&game->time_of_day);
				handle_menu_frame(game, game->bmp.menu);
			}

			break;
		}
		case OPTIONSMENU:{
			if (game->event == TIMER){
				handle_menu_frame(game, game->bmp.options);
			}
			if (game->event == MOUSE){
				cursor_update(game);
				if (game->mouse_event.lb){
					if (game->cursor.y >= 74 && game->cursor.y <= 98) {
						if (game->cursor.x >= 547  && game->cursor.x <= 567 )
							game->settings.fps = 2;
						else if (game->cursor.x >= 660  && game->cursor.x <= 680 )
							game->settings.fps = 1;
						else if (game->cursor.x >= 870  && game->cursor.x <= 890 )
							game->settings.fps = 0;
					}
					if (game->cursor.y >= 99 && game->cursor.y <= 120) {
						if (game->cursor.x >= 583  && game->cursor.x <= 603 )
							game->settings.m_sens = 0.5;
						else if (game->cursor.x >= 712  && game->cursor.x <= 732 )
							game->settings.m_sens = 1;
						else if (game->cursor.x >= 838  && game->cursor.x <= 858 )
							game->settings.m_sens = 2;
					}
					if (game->cursor.y >= 173 && game->cursor.y <= 193 && game->cursor.x >= 368  && game->cursor.x <= 389) 
						game->settings.fps_counter ^= 1;
					if (game->cursor.y >= 127 && game->cursor.y <= 147 && game->cursor.x >= 678  && game->cursor.x <= 698) 
						game->settings.page_flip ^= 1;
					if (game->cursor.y >= 150 && game->cursor.y <= 170 && game->cursor.x >= 575  && game->cursor.x <= 595){ 
						game->settings.vsync ^= 1;
						game->settings.page_flip = game->settings.vsync;	
					}
					if (game->cursor.y >= 704 && game->cursor.y <= 756 && game->cursor.x >= 13  && game->cursor.x <= 125)
						game->state = MENU;
				}
			}
			break;
		}
		
		case START_SEQUENCE:{ 
			if (game->event == TIMER){
				if (game->timers.start_seq == 3){
					game->timers.timerTick = 0;
					ship_spawn(&game->player1);
					if (game->alien.active)
						game->alien.active = false;
				}
				
				if (game->timers.start_seq == 0 && (game->timers.timerTick % 60 == 0 )) {
					game->keyboard_event = IDLING;
					start_timers(&game->timers);
					game->state = NEW_ROUND;
					break;
				}
				
				if (game->timers.timerTick % 60 == 0){
					render_seq_frame(game);
					display_frame(game->settings.page_flip, game->settings.vsync);
					game->timers.start_seq--;
				}
			}
			
			break;
		}
		
		case PLAYING:{
			static int round_delay = 0;
			playing_event_handler(game);
			
			if (game->player1.end_round){
				if (game->event == TIMER)
					round_delay++;
				
				if (round_delay >= DELAY_BETWEEN_ROUNDS){
					round_delay = 0;
					game->state = NEW_ROUND;
				}
			}
			break;
		}
		
		case NEW_ROUND:{

			if (game->player1.round < MAX_ASTEROIDS)
				game->player1.round += ASTEROID_INCREASE_RATE;
			if (game->player1.hp < PLAYER_MAX_HEALTH)
				game->player1.hp += PLAYER_HEALTH_REGENERATION;
			if (game->player1.hp > PLAYER_MAX_HEALTH)
				game->player1.hp = PLAYER_MAX_HEALTH;
			
			ast_spawn(game->asteroid_field, &game->player1);
			int random_alien_spawn = rand() % (100 - 1) + 1;
			if (random_alien_spawn > (100 - ( (game->player1.round - STARTING_ASTEROIDS) * ALIEN_SPAWN_CHANCE_INCREASE) ) )
				alien_spawn(&game->alien);
		
			game->timers.round_timer = 0;
			game->player1.invulnerability = true;
			game->state = PLAYING;

			break;
		}
		case LOSS:{
			static bool highscore;
			static bool first_frame = true;
			if (first_frame){
				game->timers.start_seq = 3;
				first_frame = false;
				highscore = verify_highscores (game->FILEPATH, game->highscores, &game->player1);
			}
			switch (game->event){
				
				case MOUSE:{
					
					cursor_update(game);		
					//Checking button clicks
					if (game->mouse_event.lb){
						if (game->cursor.x >= 227 && game->cursor.x <= 444 && game->cursor.y >= 385 && game->cursor.y <= 454) {
							game->state = START_SEQUENCE;
							first_frame = true;
							break;
						}
						else if (game->cursor.x >= 521 && game->cursor.x <= 738 && game->cursor.y >= 385 && game->cursor.y <= 454) {
							game->state = MENU;
							first_frame = true;
						}
					}
					break;
				}
				
				case TIMER:{
					render_frame(game);
					if (highscore)
						handle_menu_frame(game, game->bmp.death_screen_highscore);
					else handle_menu_frame(game, game->bmp.death_screen);
					break;
				}
				default: break;
				
			}
			break;	
		}
		
		case GAMEPAUSED:{
			game->timers.start_seq = 3;
			if (game->event == KEYBOARD){
				if (game->keyboard_event == K_ESC)
					game->state = PLAYING;
				else if (game->keyboard_event == QUIT)
					game->state = MENU;
			}
			break;
		}
		
		case CONNECTING: { mp_connect (game); break; }
		case MULTIPLAYER:{ mp_playing_event_handler(game); break; }
		
		case MULTIPLAYEROVER:{
			uint32_t lsr;
			uint8_t data;
			sys_inb( (COM1_ADDR + SP_LINE_STATUS) , &lsr);
			while (lsr & SP_RX_READY){
				sp_read_buffer(COM1_ADDR, &data);
				sys_inb( (COM1_ADDR + SP_LINE_STATUS) , &lsr);
			}
			
			sys_inb( (COM2_ADDR + SP_LINE_STATUS) , &lsr);
			while (lsr & SP_RX_READY){
				sp_read_buffer(COM2_ADDR, &data);
				sys_inb( (COM2_ADDR + SP_LINE_STATUS) , &lsr);
			}
			game->mpconnection = false;
			game->serial_event = NEWTRNM;
			if (game->player1.hp > 0)
				handle_menu_frame(game, game->bmp.mp_win_screen);
			else handle_menu_frame(game, game->bmp.mp_loss_screen);
			if (game->event == MOUSE){
				cursor_update(game);		
				//Checking button clicks
				if (game->mouse_event.lb){
					if (game->cursor.x >= 297  && game->cursor.x <= 732 && game->cursor.y >= 298 && game->cursor.y <= 380 && game->player1.hp > 0){
						game->state = MENU;
						game->mouse_event.lb = false;
					}
					else if (game->cursor.x >= 241  && game->cursor.x <= 801 && game->cursor.y >= 341 && game->cursor.y <= 424 && game->player1.hp <= 0){
						game->state = MENU;
						game->mouse_event.lb = false;
					}
				}
			}
			
			
			break;
		}

		case COMP:{
			free_xpms(game);
			free_bitmaps(game);
			return;
		}
	}
}

void playing_event_handler(game_data* game){
	switch (game->event){
		
		case KEYBOARD:{		
			ship_apply_force(&game->keyboard_event, &game->player1);		
			if (game->keyboard_event == K_ESC){
				render_frame(game);
				drawBitmap(game->bmp.pause_message, 0, 0);
				display_frame(game->settings.page_flip, game->settings.vsync);
				game->state = GAMEPAUSED;	
			}
			else if (game->keyboard_event == QUIT)
				game->state = COMP;
			break;
		}
	
		case MOUSE:{
			crossh_update(&game->mouse_event, &game->player1);
						
			if (game->mouse_event.lb && game->player1.weapon_ready)
				ship_fire_laser(&game->player1, &game->timers.player1_weapon_timer);
			
			if (game->mouse_event.rb && game->player1.jump_ready)
				ship_teleport(&game->player1, &game->timers.teleport_timer);
			
			break;
		}
			
		case TIMER:{
			/* Physics update */
			if (game->timers.timerTick % PHYSICS_TICKS == 0)
				physics_update(game);								
			/* Locked fps render */
			if (game->settings.fps)
				if (game->timers.timerTick % game->settings.fps == 0)
					handle_frame(game);					
			/* Fire rate controller */
			if (game->timers.player1_weapon_timer >= (60 / FIRE_RATE) && !(game->player1.weapon_ready) )
				game->player1.weapon_ready = true;		
			/* Jump rate controller */
			if (game->timers.teleport_timer >= (JUMP_RATE * 60) && !(game->player1.jump_ready) )
				game->player1.jump_ready = true;

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

