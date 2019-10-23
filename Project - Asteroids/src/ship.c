#include "ship.h"
#include "graphics.h"

 /* GENERAL SHIP */

void ship_spawn(player *p){
	
	/* Initiates ship's values */	
	p->cannon.x = 30;
	p->cannon.y = 20;
	
	p->pivot.x = 30;
	p->pivot.y = 0;
	
	p->port.x = -1;
	p->port.y = 0;
	
	p->starboard.x = 1;
	p->starboard.y = 0; 
	
	p->force.x = 0;
	p->force.y = 0;
	
	p->crosshair.x = 30;
	p->crosshair.y = 50;
	
	p->hp = PLAYER_MAX_HEALTH;
	p->hit_radius = SHIP_HITRADIUS;
	p->score = 0;
	p->end_round = false;
	p->round = STARTING_ASTEROIDS;
	p->invulnerability = false;
	p->jump_ready = false;
		
	for (int i = 0; i < AMMO; i++){
		p->lasers[i].active = false;
	}
}

void ship_warp(player *p){
	
	/* Warps cannon point */
	if (p->cannon.x > math_h_positive_bound)
		p->cannon.x -= hres;
	
	else if (p->cannon.x < math_h_negative_bound)
		p->cannon.x +=	hres;
	
	
	if (p->cannon.y > math_v_positive_bound)
		p->cannon.y -=vres;
	
	else if (p->cannon.y < math_v_negative_bound)
		p->cannon.y += vres;
	
	/* Warps pivot point and rotates side versors */
	if (p->pivot.x > math_h_positive_bound){
		p->pivot.x -= hres;
		mvector_rotate(&p->port, 180);
		mvector_rotate(&p->starboard, 180);	
	}
	
	else if (p->pivot.x < math_h_negative_bound){
		p->pivot.x += hres;
		mvector_rotate(&p->port, 180);
		mvector_rotate(&p->starboard, 180);	
	}

	
	if (p->pivot.y > math_v_positive_bound){
		p->pivot.y -= vres;
		mvector_rotate(&p->port, 180);
		mvector_rotate(&p->starboard, 180);	
	}
	
	else if (p->pivot.y < math_v_negative_bound){
		p->pivot.y += vres;
		mvector_rotate(&p->port, 180);
		mvector_rotate(&p->starboard, 180);	
	}
}

void ship_teleport (player *p, unsigned int *timer){
	
	/* Teleports ship to a random location avoiding edges, resets jump timer */
	int random_x = rand() % 400;	
	int	random_y = rand() % 300;

	uint8_t random_xsign = rand() % 10;
	uint8_t random_ysign = rand() % 10;
	
	int current_x = p->pivot.x;
	int current_y = p->pivot.y;
	if (random_xsign >= 5)
		random_x *= -1;
	if (random_ysign >= 5)
		random_y *= -1;
	
	p->pivot.x = random_x;
	p->pivot.y = random_y;
	p->cannon.x += random_x - current_x;
	
	p->cannon.y += random_y - current_y;
	p->jump_ready = false;
	*timer = 0;
		
}

void ship_apply_force (kb_evt_tp *keyboard_event, player *p){
	
	/* Creates necessary versors */
	mvector vcannon = mvector_create (p->pivot, p->cannon);
	mvector cannon_versor = mvector_get_versor(&vcannon);
	mvector port_th = p->port;
	mvector starboard_th = p->starboard;
	
	/* Applies force based on keyboard input and accelaration values in "macros.h" */
	switch (*keyboard_event) {
		case MAIN_THRUSTER:{
			mvector_multiplication(&cannon_versor, MAIN_THRUSTER_ACCELARATION);
			mvector_add (&p->force, &cannon_versor);
			break;
		}
		case PORT_THRUSTER:{
			mvector_multiplication(&port_th, PORT_THRUSTER_ACCELARATION);
			mvector_add (&p->force, &port_th);
			break;
		}
		case STARBOARD_THRUSTER:{
			mvector_multiplication(&starboard_th, STARBOARD_THRUSTER_ACCELARATION);
			mvector_add (&p->force, &starboard_th);
			break;
		}
		case REVERSE_THRUSTER:{
			mvector_multiplication(&cannon_versor, MAIN_REVERSE_ACCELARATION);
			mvector_subtract (&p->force, &cannon_versor);
			break;
		}
		default: break;	
	}
	
	/* Limits the force vector, and effectively the ship's velocity, to the maximum velocity value in "macros.h" */
	mvector_limit(&p->force, THRUSTERS_MAXIMUM_VELOCITY);
}

void ship_fire_laser (player *p, unsigned int *timer){
	
	/* Activates an inactive laser from the laser struct, and gives it the same direction as the cannon versor */
	for (unsigned int i = 0; i < AMMO; i++)
		if (!p->lasers[i].active){
			mvector vcannon = mvector_create(p->pivot, p->cannon);
			mvector cannon_versor = mvector_get_versor(&vcannon);
			mvector_multiplication(&cannon_versor, LASER_VELOCITY);
			p->lasers[i].active = true;
			p->lasers[i].position = p->cannon;
			p->lasers[i].force = cannon_versor;
			p->weapon_ready = false;
			*timer = 0;
			break;
		}
}

void ship_update(player *p) {
	
	/* Updates ship's position */
	p->cannon.x += p->force.x;
	p->cannon.y += p->force.y;
	p->pivot.x += p->force.x;
	p->pivot.y += p->force.y;
	
	/* Warps ship to the other end when crossing bounds */
	ship_warp(p);
	
	/* Rotates ship, avoids rotating when cursor is near the center of the ship */
	mvector vcannon = mvector_create (p->pivot, p->cannon);
	mvector vmouse = mvector_create (p->pivot, p->crosshair);
	
	if (mvector_magnitude (&vmouse) > 5){
		double degrees = mvector_angle(&vmouse) - mvector_angle(&vcannon);
		mvector_rotate(&vcannon, degrees);
		mvector_limit(&vcannon, 20);
		mvector_rotate(&p->port, degrees);
		mvector_rotate(&p->starboard, degrees);
	}
	
	p->cannon.x = p->pivot.x + vcannon.x;
	p->cannon.y = p->pivot.y + vcannon.y;
	
	/* Destroys out of bounds active lasers */
	for (unsigned int i = 0; i < AMMO; i++){
		if (p->lasers[i].active)
			p->lasers[i].position.x += p->lasers[i].force.x;
			p->lasers[i].position.y += p->lasers[i].force.y;
			if (p->lasers[i].position.x >= math_h_positive_bound || p->lasers[i].position.x <= math_h_negative_bound)
				p->lasers[i].active = false;
			if (p->lasers[i].position.y >= math_v_positive_bound || p->lasers[i].position.y <= math_v_negative_bound)
				p->lasers[i].active = false;
	}
}

void crossh_update(struct packet *mouse_event, player *p){
	
	/* Updates the crosshair's position */					
	p->crosshair.x += mouse_event->delta_x * MOUSE_SENSITIVITY;
	p->crosshair.y += mouse_event->delta_y * MOUSE_SENSITIVITY;
	
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


/* ALIEN SHIP */

void alien_spawn(player *p){
	
	/*Initiates alien ships values, gives it random movement */
	p->force.x += (double)rand()/RAND_MAX;
	p->force.y += (double)rand()/RAND_MAX;
	
	int random_xsign = rand() % 10;
	int random_ysign = rand() % 10;

	if (random_xsign >= 5){
		p->force.x *= -1;
	}
		
	if (random_ysign >= 5) {
		p->force.y *= -1;	
	}
	
	p->pivot.x = rand() % math_h_positive_bound;
	p->pivot.y = rand() % math_v_positive_bound;
	
	p->cannon.x = p->pivot.x;
	p->cannon.y = p->pivot.y + 20;
	
	p->port.x = -1;
	p->port.y = 0;
	
	p->starboard.x = 1;
	p->starboard.y = 0; 
	
	p->hp = ALIEN_MAX_HEALTH;
	p->hit_radius = ALIEN_HIT_RADIUS;
	p->active = true;
		
	for (int i = 0; i < AMMO; i++){
		p->lasers[i].active = false;
	}
}

void alien_update(player *alien, player *player1, game_timers *timers){
	
	/* Updates alien ships position */
	alien->cannon.x += alien->force.x;
	alien->cannon.y += alien->force.y;
	alien->pivot.x += alien->force.x;
	alien->pivot.y += alien->force.y;
	
	/* Warps alien ship to the other end when crossing bounds */
	ship_warp(alien);
	
	/* Rotates ship, so it tracks the player's ship */
	mvector vcannon = mvector_create (alien->pivot, alien->cannon);
	mvector vships = mvector_create (alien->pivot, player1->pivot);
	double ship_degrees = mvector_angle(&vships) - mvector_angle(&vcannon);
	if (ship_degrees){
			if (ship_degrees < 0)
				mvector_rotate(&vcannon, - ALIEN_ROTATION_SPEED);
			else mvector_rotate(&vcannon, + ALIEN_ROTATION_SPEED);
	}
	mvector_limit(&vcannon, 20);
	alien->cannon.x = alien->pivot.x + vcannon.x;
	alien->cannon.y = alien->pivot.y + vcannon.y;
	
	/* Fires lasers when available */
	if (timers->alien_weapon_timer >= (60 / ALIEN_FIRE_RATE))
		ship_fire_laser(alien, &timers->alien_weapon_timer);
	
	/* Destroys out of bounds lasers */
	for (unsigned int i = 0; i < AMMO; i++){
		if (alien->lasers[i].active)
			alien->lasers[i].position.x += alien->lasers[i].force.x;
			alien->lasers[i].position.y += alien->lasers[i].force.y;
			if (alien->lasers[i].position.x >= math_h_positive_bound || alien->lasers[i].position.x <= math_h_negative_bound)
				alien->lasers[i].active = false;
			if (alien->lasers[i].position.y >= math_v_positive_bound || alien->lasers[i].position.y <= math_v_negative_bound)
				alien->lasers[i].active = false;
	}
}

void alien_collision(player *alien, player *player1, game_timers *timers){
		
	/* Player laser to alien ship collision */
	for (unsigned int j = 0; j < AMMO; j++){
		if (player1->lasers[j].active){
			mvector v_ast_laser = mvector_create(player1->lasers[j].position, alien->pivot);
			double distance = mvector_magnitude(&v_ast_laser);
			if (distance <= alien->hit_radius){
				alien->hp -= PLAYER_LASER_DAMAGE;
				player1->lasers[j].active = false;
				if (alien->hp <= 0){
					timers->alien_death_timer = ALIEN_DEATH_DURATION * 60;
					alien->active = false;
					player1->score += 400;
				}
			}
		}
	}
		
	/* Alien laser to player ship collision */
	for (unsigned int j = 0; j < AMMO; j++){
		if (alien->lasers[j].active){
			mvector v_ast_laser = mvector_create(alien->lasers[j].position, player1->pivot);
			double distance = mvector_magnitude(&v_ast_laser);
			if (distance <= player1->hit_radius){
				player1->hp -= 20;
				alien->lasers[j].active = false;
			}
		}
	}
	
	/* Alien ship to player ship collision, this kills the player ship */
	mvector v_ast_ship = mvector_create(player1->pivot, alien->pivot);
	double dist = mvector_magnitude(&v_ast_ship);
	int total_radius = player1->hit_radius + alien->hit_radius;
	if (total_radius > dist)
		player1->hp -= player1->hp;
}


/* HIGHSCORES */

int load_highscores(char FILEPATH[], unsigned int highscores[]){
	
	/* Opens "highscores.txt" and loads highscores to the highscore array */
	FILE *fptr;
	fptr = fopen ( get_filepath(FILEPATH, "highscores.txt"), "r" );
	if (fptr == NULL)
		return 0;
	
	for (int i = 0; i < 5; i++)
		fscanf(fptr, "%d", &highscores[i]);

	return 1;
}

void save_highscores(char FILEPATH[], unsigned int highscores[]){
	
	/* Opens "highscores.txt" and saves highscores to the file */
	FILE *fptr;

	fptr = fopen ( get_filepath(FILEPATH, "highscores.txt"), "w" );
	for (int i = 0; i < 5; i++)
		fprintf(fptr, "%d\n", highscores[i]);
	
	fclose(fptr);
}

int verify_highscores (char FILEPATH[], unsigned int highscores[], player *player1){
	
	/* Check if score is a highscore */
	if (player1->score > highscores[4]){
		highscores[4] = player1->score;
		
		/* Sort highscore array by highest to loweat if a new highscore is found */
		for (int i = 0; i < 5; i++){	
			for (int j = i + 1; j < 5; j++){
				if (highscores[i] < highscores[j]){
					unsigned int tempvar = highscores[i];
					highscores[i] = highscores[j];
					highscores[j] = tempvar;
				}
			}
		}
		/* Save highscores if a new highscore is found */
		save_highscores(FILEPATH, highscores);
		return 1;
	}
	
	return 0;
}
