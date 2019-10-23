#include "vcard.h"
#include <liblm.h>
#include "macros.h"
#include "graphics.h"
#include "pixmaps.h"


int vbe_mode_info (uint16_t mode, vbe_ModeInfoBlock *mode_info){
	
	/*
		Input: 
			AX = 0x4F01 - Return VBE Mode Information
			CX = Mode
			ES:DI = Pointer to vbe_ModeInfoBlock
		Output:
			AX = Return Status
	*/
	
	mmap_t map;
	struct reg86u rg;
	memset(&rg, 0, sizeof(rg));
	
	/* Initialize low memory */
	
	if (lm_init(false) == NULL)
		return 1;
	
	if (lm_alloc(sizeof(*mode_info), &map) == NULL)
		return 1;
	
	/* Set ASM variables */
	
	rg.u.w.ax = get_vbe_info;
	rg.u.w.cx = mode;
	rg.u.w.es = PB2BASE (map.phys);
	rg.u.w.di = PB2OFF (map.phys);
	rg.u.b.intno = video_card_bios;
	
	/* Get mode information */
	
	if( sys_int86(&rg) != OK )
		return 1;
	
	if (rg.u.w.ax != vbe_success_return)
		return 1;
	
	/* Copy information to vbe_ModeInfoBlock */
	
	memcpy(mode_info,map.virt, 256);
	
	/* Free allocated memory */
	
	if (lm_free (&map) == 0)
		return 1;
	
	return 0;
}

void* vmode_init (uint16_t mode) {
	
	/*
		Input: 
			AX = 0x4F02 - Set Video Mode
			BX = Mode - 0x105
		Output:
			AX = Return Status
		Notes:
			Bit 14 of mode is set to use a flat frame buffer model
			Return status is not used as it often reports incorrectly in virtual machine mode
			Using 1024x768 Indexed 8 bit mode for this game
	*/
	
	void *video_mem;
	vbe_ModeInfoBlock mode_info;
	struct reg86u rg;
	memset(&rg, 0, sizeof(rg));	
	unsigned int vram_base;
	struct minix_mem_range mr;
	int r;
	
	/* Initialize low memory and get vbe mode information */
	
	if (vbe_mode_info(mode, &mode_info))
		panic("Failed to get vbe mode info\n");
	
	pixel_bits = mode_info.BitsPerPixel;
	gcard_phys_address = mode_info.PhysBasePtr;
	
	/* Set ASM variables */
	
	mode =  0x1<<14|mode;
	rg.u.w.ax = set_vbe_mode;
	rg.u.w.bx = mode;
	rg.u.b.intno = video_card_bios;

	/* Allow memory mapping */
	
	vram_base = mode_info.PhysBasePtr;
	vram_size = hres * vres * (pixel_bits / 8);
	
	mr.mr_base = (phys_bytes) vram_base;
	mr.mr_limit = mr.mr_base + (vram_size * 2);
	
	if ( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		panic("sys_privctl (ADD_MEM) failed: %d\n", r);
	
	/* Map memory */
	
	video_mem = vm_map_phys(SELF, (void *)mr.mr_base, (vram_size * 2));

	if (video_mem == MAP_FAILED)
		panic ("Couldn't map video memory\n");
	
	/* Set global primary and seconday buffer addresses */
	
	gcard_v_address = video_mem;
	pixel_buffer = (uint8_t *) malloc(vram_size);
	
	/* Initialize video mode */
	
	if( sys_int86(&rg) != OK )
		panic ("Failed to set mode\n");
	
	if (rg.u.w.ax != vbe_success_return)
		panic ("Vbe set mode function call unsucessful\n");
	
	return pixel_buffer;
}

int vmode_exit() {
	
	/*
		Input: 
			AX = 0x0003 - Return to Minix's text mode
		Output:
			AX = Return Status
	*/
	
	struct reg86u rg;
	memset(&rg, 0, sizeof(rg));	

	rg.u.w.ax = minix_text_mode;
	rg.u.b.intno = video_card_bios;

	if( sys_int86(&rg) != OK ){
		panic ("Failed to set mode\n");
		return 1;
	}
	
	return 0;
}

int vbe_switch_display_start(bool flip, bool vsync){

	struct reg86u rg;
	memset(&rg, 0, sizeof(rg));	
	if (flip){
		rg.u.b.intno = video_card_bios;
		rg.u.w.ax = 0x4F07;
		rg.u.w.cx = PB2BASE (gcard_phys_address);
		rg.u.w.dx = PB2OFF (gcard_phys_address) + vres;
		rg.u.b.bh = 0x00;
		if (vsync)
			rg.u.b.bl = 0x80;
		else rg.u.b.bl = 0x00;
	
		if( sys_int86(&rg) != OK )
			return -1;
	}
	else {
		rg.u.b.intno = video_card_bios;
		rg.u.w.ax = 0x4F07;
		rg.u.w.cx = PB2BASE (gcard_phys_address);
		rg.u.w.dx = PB2OFF (gcard_phys_address);
		rg.u.b.bh = 0x00;
		if (vsync)
			rg.u.b.bl = 0x80;
		else rg.u.b.bl = 0x00;
	
		if( sys_int86(&rg) != OK )
			return -1;
	}
	return 0;
}
int draw_pixel(int x, int y, uint32_t color){
	uint8_t *v_address = pixel_buffer;
	
	if (x > hres - 1 || y > vres -1 || x < 0 || y < 0 ){
		return 1;
	}
	
	unsigned int pixel_bytes;
	if (pixel_bits == 15)
		pixel_bytes = 2;
	else pixel_bytes = pixel_bits/8;
	

	v_address += ((hres * y) + x) * pixel_bytes;
	
	for (unsigned int i = 0; i < pixel_bytes; i++){
		*v_address = color >> (8 * i);
		v_address++;
	}
	
	return 0;
}

void draw_digit(unsigned int num, int x, int y, game_data *game)
{
	switch (num) {
		case 0: draw_pixmap(&game->xpm.n_zero, x, y, false); break;
		case 1:	draw_pixmap(&game->xpm.n_one, x, y, false); break;
		case 2: draw_pixmap(&game->xpm.n_two, x, y, false); break;
		case 3: draw_pixmap(&game->xpm.n_three, x, y, false); break;
		case 4: draw_pixmap(&game->xpm.n_four, x, y, false); break;
		case 5: draw_pixmap(&game->xpm.n_five, x, y, false); break;
		case 6: draw_pixmap(&game->xpm.n_six, x, y, false); break;
		case 7: draw_pixmap(&game->xpm.n_seven, x, y, false); break;
		case 8: draw_pixmap(&game->xpm.n_eight, x, y, false); break;
		case 9: draw_pixmap(&game->xpm.n_nine, x, y, false); break;
			
		default: break;
	}
}

void draw_large_digit(unsigned int num, int x, int y, game_data *game) {
	
	switch (num) {
		case 1:	draw_pixmap(&game->xpm.n_one_large, x, y, false); break;
		case 2: draw_pixmap(&game->xpm.n_two_large, x, y, false); break;
		case 3: draw_pixmap(&game->xpm.n_three_large, x, y, false); break;		
		default: break;
	}
}

void draw_number (int number, int x, int y, game_data *game) {
	
	int temp_number = number;
	int size = 0;
	int reverse_number = 0;
	
	while (true){
		size++;
		reverse_number = reverse_number * 10 + (temp_number % 10);
		temp_number = floor(temp_number / 10);
		if (temp_number == 0)
			break;
	}
	
	for (int i = 0; i < size; i++) {
		draw_digit(reverse_number % 10, x, y, game);
		reverse_number = floor(reverse_number / 10);
		x += 10;
	}
}

void draw_time_of_day (game_data *game, int x, int y){
	//Hours
	if (game->time_of_day.hours > 9)
		draw_number(game->time_of_day.hours, x, y, game);
	else {
		draw_number(0, x, y, game);
		x += 10;
		draw_number(game->time_of_day.hours, x, y, game);
		x -= 10;
	}
	x += 20;
	draw_pixmap(&game->xpm.n_colon, x, y, false);
	x += 10;
	
	if (game->time_of_day.minutes > 9)
		draw_number(game->time_of_day.minutes, x, y, game);
	else {
		draw_number(0, x, y, game);
		x += 10;
		draw_number(game->time_of_day.minutes, x, y, game);
		x -= 10;
	}
	
	x += 20;
	draw_pixmap(&game->xpm.n_colon, x, y, false);
	x += 10;
	
	if (game->time_of_day.seconds > 9)
		draw_number(game->time_of_day.seconds, x, y, game);
	else {
		draw_number(0, x, y, game);
		x += 10;
		draw_number(game->time_of_day.seconds, x, y, game);
	}
	
}

void show_splash(game_data *game){
	drawBitmap(game->bmp.splash, 0, 0);
	display_frame(game->settings.page_flip, game->settings.vsync);
	sleep(2);
	display_frame(game->settings.page_flip, game->settings.vsync);
	drawBitmap(game->bmp.menu, 0, 0);
	draw_pixmap(&game->xpm.cursor, hres/2, vres/2, false);
	display_frame(game->settings.page_flip, game->settings.vsync);
}


void render_frame(game_data *game){
	
	drawBitmap(game->bmp.game_background, 0, 0);
	
	mpoint ws_mouse = vector_translate_gfx (&game->player1.crosshair, 1024, 768);
	draw_pixmap(&game->xpm.crosshair, ws_mouse.x, ws_mouse.y, 0);
							
	switch (game->keyboard_event){
	
		case MAIN_THRUSTER:{
			draw_ship(&game->xpm.ship_blue_bt, &game->player1);
			break;
		}
		case PORT_THRUSTER:{
			draw_ship(&game->xpm.ship_blue_st, &game->player1);
			break;
		}
		case STARBOARD_THRUSTER:{
			draw_ship(&game->xpm.ship_blue_pt, &game->player1);
			break;
		}
		case IDLING:{
			draw_ship(&game->xpm.ship_blue, &game->player1);
			break;
		}
		default: {
			draw_ship(&game->xpm.ship_blue, &game->player1);
			break;
		}
			
	}
	
	//Draw lasers
	for (int i = 0; i < AMMO; i++){
		if (game->player1.lasers[i].active){
			mpoint ws_laser = vector_translate_gfx (&game->player1.lasers[i].position, 1024, 768);
			draw_pixmap(&game->xpm.blue_laser, ws_laser.x, ws_laser.y, 0);
		}
	}
	
	//Draw Asteroids
	for (int i = 0; i < MAX_ASTEROIDS; i++){
		mpoint ws_ast = vector_translate_gfx (&game->asteroid_field[i].position, 1024, 768);
		//Active asteroids
		if (game->asteroid_field[i].active){
			if (game->asteroid_field[i].size == MEDIUM)
				draw_ast(&game->asteroid_field[i], &game->xpm.asteroid_medium);
			else draw_ast(&game->asteroid_field[i], &game->xpm.asteroid_large);
			
		}
		//Destruction animation
		else { 
			Bitmap *temp;
			if (game->asteroid_field[i].size == MEDIUM)
				temp = game->bmp.medium_score;
			else temp = game->bmp.large_score;
			
			if(game->asteroid_field[i].death_timer > 20){
				draw_pixmap (&game->xpm.asteroid_dest1, ws_ast.x, ws_ast.y, false);
				drawBitmap(temp, ws_ast.x + 10, ws_ast.y - 35);
			}
			else if(game->asteroid_field[i].death_timer > 10){
				draw_pixmap (&game->xpm.asteroid_dest2, ws_ast.x, ws_ast.y, false);
				drawBitmap(temp, ws_ast.x + 10, ws_ast.y - 35);
			}
			else if (game->asteroid_field[i].death_timer > 0){
				draw_pixmap (&game->xpm.asteroid_dest3, ws_ast.x, ws_ast.y, false);
				drawBitmap(temp, ws_ast.x + 10, ws_ast.y - 35);
			}
		}

	}

	//Draw FPS counter
	if (game->settings.fps_counter){
		drawBitmap(game->bmp.fps_header, 912, 3);
		draw_number (game->timers.frames_per_second, 970, 17, game);
	}
	
	//Draw Score
	drawBitmap(game->bmp.score_header, 10, 4);
	draw_number (game->player1.score, 80, 17, game);
	
	//Draw_HP
	if (game->player1.hp > 30)
		drawBitmap(game->bmp.hp_header, 130, 4);
	else drawBitmap(game->bmp.hp_header_low, 130, 4);
	draw_number (game->player1.hp, 175, 17, game);
	
	//Draw Teleport Header
	if (game->player1.jump_ready)
		drawBitmap(game->bmp.teleport_ready_header, 220, 4);
	else drawBitmap(game->bmp.teleport_not_ready_header, 220, 4);
	
	draw_alien(game);
}

int mp_render_frame(game_data *game){

	drawBitmap(game->bmp.game_background, 0, 0);
	
	mpoint ws_mouse = vector_translate_gfx (&game->player1.crosshair, 1024, 768);
	draw_pixmap(&game->xpm.crosshair, ws_mouse.x, ws_mouse.y, 0);			
	switch (game->keyboard_event){
	
		case MAIN_THRUSTER:{
			draw_ship(&game->xpm.ship_blue_bt, &game->player1);
			break;
		}
		case IDLING:{
			draw_ship(&game->xpm.ship_blue, &game->player1);
			break;
		}
		default: {
			draw_ship(&game->xpm.ship_blue, &game->player1);
			break;
		}
			
	}
	
	switch (game->p2_keyboard_event){
	
		case MAIN_THRUSTER:{
			draw_ship(&game->xpm.ship_red_bt, &game->player2);
			break;
		}
		case IDLING:{
			draw_ship(&game->xpm.ship_red, &game->player2);
			break;
		}
		default: {
			draw_ship(&game->xpm.ship_red, &game->player2);
			break;
		}
			
	}
	
	//Draw lasers
	for (unsigned int i = 0; i < AMMO; i++){
		if (game->player1.lasers[i].active){
			mpoint ws_laser = vector_translate_gfx (&game->player1.lasers[i].position, 1024, 768);
			draw_pixmap(&game->xpm.blue_laser, ws_laser.x, ws_laser.y, false);
		}
		if (game->player2.lasers[i].active){
			mpoint ws_laser2 = vector_translate_gfx (&game->player2.lasers[i].position, 1024, 768);
			draw_pixmap(&game->xpm.red_laser, ws_laser2.x, ws_laser2.y, false);
		}
	}
	
	//Draw FPS
	if (game->settings.fps_counter){
		drawBitmap(game->bmp.fps_header, 912, 3);
		draw_number (game->timers.frames_per_second, 970, 17, game);
	}
	
	if (game->host){
		//Draw Player 1 HP
		drawBitmap(game->bmp.hp_header, 880, 714);
		draw_number (game->player2.hp, 930, 727, game);
	
		//Draw Player 2 HP
		drawBitmap(game->bmp.hp_header, 80, 714);
		draw_number (game->player1.hp, 130, 727, game);
	}
	else {
		//Draw Player 1 HP
		drawBitmap(game->bmp.hp_header, 880, 714);
		draw_number (game->player1.hp, 930, 727, game);
	
		//Draw Player 2 HP
		drawBitmap(game->bmp.hp_header, 80, 714);
		draw_number (game->player2.hp, 130, 727, game);
	}
	
	return 0;
}

void render_seq_frame(game_data *game){
	
	drawBitmap(game->bmp.game_background, 0, 0);
	draw_ship(&game->xpm.ship_blue, &game->player1);
	draw_large_digit(game->timers.start_seq, 512, 304, game);
}

void display_frame(bool page_flip, bool vsync){
	static bool flip = false;
	
	if (page_flip){
		
		if (flip)
			memcpy(gcard_v_address + vram_size, pixel_buffer, vram_size);
		else memcpy(gcard_v_address, pixel_buffer, vram_size);
		
		vbe_switch_display_start(flip, vsync);
		flip ^= 1;
	}
	else memcpy(gcard_v_address, pixel_buffer, vram_size);
	
	memset(pixel_buffer, 0, vram_size);	
}

void draw_ship (pixmap *xpm, player *p){
	double degrees;
	
	mvector vmouse = mvector_create (p->pivot, p->crosshair);
	mvector vcannon = mvector_create (p->pivot, p->cannon);
	if (mvector_magnitude (&vmouse) > 5)
		degrees = mvector_angle(&vmouse) - 90; 
	else degrees = mvector_angle(&vcannon) - 90; 
	
	mpoint ws_pivot = vector_translate_gfx (&p->pivot, 1024, 768);
	
	draw_pixmap(xpm, ws_pivot.x, ws_pivot.y, degrees);
}

void draw_alien(game_data *game){
	if (game->alien.active){
		mpoint ws_pivot = vector_translate_gfx (&game->alien.pivot, 1024, 768);
		mvector vcannon = mvector_create (game->alien.pivot, game->alien.cannon);
		draw_pixmap(&game->xpm.alien_ship, ws_pivot.x, ws_pivot.y,  mvector_angle(&vcannon) - 90);
	
		//Draw alien lasers
		for (int i = 0; i < AMMO; i++){
			if (game->alien.lasers[i].active){
				mpoint ws_laser = vector_translate_gfx (&game->alien.lasers[i].position, 1024, 768);
				draw_pixmap(&game->xpm.red_laser, ws_laser.x, ws_laser.y, 0);
			}
		}
	}
	else if (game->timers.alien_death_timer > 0){
		mpoint ws_pivot = vector_translate_gfx (&game->alien.pivot, 1024, 768);
		if(game->timers.alien_death_timer > 20){
			draw_pixmap (&game->xpm.asteroid_dest1, ws_pivot.x, ws_pivot.y, false);
			drawBitmap(game->bmp.alien_score, ws_pivot.x + 10, ws_pivot.y - 35);
		}
		else if(game->timers.alien_death_timer > 10){
			draw_pixmap (&game->xpm.asteroid_dest2, ws_pivot.x, ws_pivot.y, false);
			drawBitmap(game->bmp.alien_score, ws_pivot.x + 10, ws_pivot.y - 35);
		}
		else {
			draw_pixmap (&game->xpm.asteroid_dest3, ws_pivot.x, ws_pivot.y, false);
			drawBitmap(game->bmp.alien_score, ws_pivot.x + 10, ws_pivot.y - 35);
		}
	}
}


void draw_ast(asteroid *ast, pixmap *xpm){

	mpoint ws_ast = vector_translate_gfx (&ast->position, 1024, 768);	
	draw_pixmap (xpm, ws_ast.x, ws_ast.y, ast->degrees);

	if (ast->degrees >= 360)
		ast->degrees -= 360;
	else ast->degrees++;
	
}

void load_xpms(game_data *game){

	game->xpm.ship_blue.map = read_pixmap24(pix_ship_blue, &game->xpm.ship_blue.width, &game->xpm.ship_blue.height);
	game->xpm.ship_blue_bt.map = read_pixmap24(pix_ship_blue_bt, &game->xpm.ship_blue_bt.width, &game->xpm.ship_blue_bt.height);
	game->xpm.ship_blue_pt.map = read_pixmap24(pix_ship_blue_pt, &game->xpm.ship_blue_pt.width, &game->xpm.ship_blue_pt.height);
	game->xpm.ship_blue_st.map = read_pixmap24(pix_ship_blue_st, &game->xpm.ship_blue_st.width, &game->xpm.ship_blue_st.height);
	
	game->xpm.ship_red.map = read_pixmap24(pix_ship_red, &game->xpm.ship_red.width, &game->xpm.ship_red.height);
	game->xpm.ship_red_bt.map = read_pixmap24(pix_ship_red_bt, &game->xpm.ship_red_bt.width, &game->xpm.ship_red_bt.height);
	game->xpm.ship_red_pt.map = read_pixmap24(pix_ship_red_pt, &game->xpm.ship_red_pt.width, &game->xpm.ship_red_pt.height);
	game->xpm.ship_red_st.map = read_pixmap24(pix_ship_red_st, &game->xpm.ship_red_st.width, &game->xpm.ship_red_st.height);
	
	game->xpm.alien_ship.map = read_pixmap24(pix_alien_ship, &game->xpm.alien_ship.width, &game->xpm.alien_ship.height);
	
	game->xpm.asteroid_medium.map = read_pixmap24(pix_asteroid_medium, &game->xpm.asteroid_medium.width, &game->xpm.asteroid_medium.height);
	game->xpm.asteroid_large.map = read_pixmap24(pix_asteroid_large, &game->xpm.asteroid_large.width, &game->xpm.asteroid_large.height);
	game->xpm.asteroid_dest1.map = read_pixmap24(pix_asteroid_destroyed_1, &game->xpm.asteroid_dest1.width, &game->xpm.asteroid_dest1.height);
	game->xpm.asteroid_dest2.map = read_pixmap24(pix_asteroid_destroyed_2, &game->xpm.asteroid_dest2.width, &game->xpm.asteroid_dest2.height);
	game->xpm.asteroid_dest3.map = read_pixmap24(pix_asteroid_destroyed_3, &game->xpm.asteroid_dest3.width, &game->xpm.asteroid_dest3.height);
	
	game->xpm.cursor.map = read_pixmap24(pix_menu_cursor, &game->xpm.cursor.width, &game->xpm.cursor.height);
	game->xpm.crosshair.map = read_pixmap24(pix_crosshair, &game->xpm.crosshair.width, &game->xpm.crosshair.height);
	
	game->xpm.blue_laser.map = read_pixmap24(pix_laser, &game->xpm.blue_laser.width, &game->xpm.blue_laser.height);
	game->xpm.red_laser.map = read_pixmap24(pix_laser_red, &game->xpm.red_laser.width, &game->xpm.red_laser.height);
	
	game->xpm.n_colon.map = read_pixmap24(colon, &game->xpm.n_colon.width, &game->xpm.n_colon.height);
	game->xpm.n_zero.map = read_pixmap24(zero, &game->xpm.n_zero.width, &game->xpm.n_zero.height);
	game->xpm.n_one.map = read_pixmap24(one, &game->xpm.n_one.width, &game->xpm.n_one.height);
	game->xpm.n_two.map = read_pixmap24(two, &game->xpm.n_two.width, &game->xpm.n_two.height);
	game->xpm.n_three.map = read_pixmap24(three, &game->xpm.n_three.width, &game->xpm.n_three.height);
	game->xpm.n_four.map = read_pixmap24(four, &game->xpm.n_four.width, &game->xpm.n_four.height);
	game->xpm.n_five.map = read_pixmap24(five, &game->xpm.n_five.width, &game->xpm.n_five.height);
	game->xpm.n_six.map = read_pixmap24(six, &game->xpm.n_six.width, &game->xpm.n_six.height);
	game->xpm.n_seven.map = read_pixmap24(seven, &game->xpm.n_seven.width, &game->xpm.n_seven.height);
	game->xpm.n_eight.map = read_pixmap24(eight, &game->xpm.n_eight.width, &game->xpm.n_eight.height);
	game->xpm.n_nine.map = read_pixmap24(nine, &game->xpm.n_nine.width, &game->xpm.n_nine.height);
	game->xpm.n_one_large.map = read_pixmap24(one_large, &game->xpm.n_one_large.width, &game->xpm.n_one_large.height);
	game->xpm.n_two_large.map = read_pixmap24(two_large, &game->xpm.n_two_large.width, &game->xpm.n_two_large.height);
	game->xpm.n_three_large.map = read_pixmap24(three_large, &game->xpm.n_three_large.width, &game->xpm.n_three_large.height);
	
}

void free_xpms(game_data *game){
	
	free(game->xpm.ship_blue.map);
	free(game->xpm.ship_blue_bt.map);
	free(game->xpm.ship_blue_pt.map);
	free(game->xpm.ship_blue_st.map);
	free(game->xpm.ship_red.map);
	free(game->xpm.ship_red_bt.map);
	free(game->xpm.ship_red_pt.map);
	free(game->xpm.ship_red_st.map);
	free(game->xpm.alien_ship.map);
	free(game->xpm.asteroid_medium.map);
	free(game->xpm.asteroid_large.map);
	free(game->xpm.asteroid_dest1.map);
	free(game->xpm.asteroid_dest2.map);
	free(game->xpm.asteroid_dest3.map);
	free(game->xpm.cursor.map);
	free(game->xpm.crosshair.map);
	free(game->xpm.blue_laser.map);
	free(game->xpm.red_laser.map);
	free(game->xpm.n_colon.map);
	free(game->xpm.n_zero.map);
	free(game->xpm.n_one.map);
	free(game->xpm.n_two.map);
	free(game->xpm.n_three.map);
	free(game->xpm.n_four.map);
	free(game->xpm.n_five.map);
	free(game->xpm.n_six.map);
	free(game->xpm.n_seven.map);
	free(game->xpm.n_eight.map);
	free(game->xpm.n_nine.map);
	free(game->xpm.n_one_large.map);
	free(game->xpm.n_two_large.map);
	free(game->xpm.n_three_large.map);
	
}

void load_bitmaps(game_data *game){
	
	game->bmp.menu = loadBitmap( get_filepath(game->FILEPATH,"bmps/menu.bmp") );
	game->bmp.options = loadBitmap( get_filepath(game->FILEPATH,"bmps/options.bmp") );
	game->bmp.boxticked = loadBitmap( get_filepath(game->FILEPATH,"bmps/boxticked.bmp") );
	game->bmp.game_background = loadBitmap( get_filepath(game->FILEPATH, "bmps/gamebackground.bmp") );
	game->bmp.host_connecting = loadBitmap( get_filepath(game->FILEPATH, "bmps/hostconnecting.bmp") );	
	game->bmp.client_connecting = loadBitmap( get_filepath(game->FILEPATH, "bmps/clientconnecting.bmp") );	
	game->bmp.connected = loadBitmap( get_filepath(game->FILEPATH, "bmps/connected.bmp") );
	game->bmp.pause_message = loadBitmap( get_filepath(game->FILEPATH, "bmps/pausemessage.bmp") );
	game->bmp.splash = loadBitmap( get_filepath(game->FILEPATH, "bmps/splash.bmp") );
	game->bmp.death_screen = loadBitmap( get_filepath(game->FILEPATH, "bmps/deathscreen.bmp") );
	game->bmp.death_screen_highscore = loadBitmap( get_filepath(game->FILEPATH, "bmps/deathscreenhighscore.bmp") );
	game->bmp.score_header = loadBitmap( get_filepath(game->FILEPATH, "bmps/score_header.bmp") );
	game->bmp.hp_header = loadBitmap( get_filepath(game->FILEPATH, "bmps/hp_header.bmp") );
	game->bmp.hp_header_low = loadBitmap( get_filepath(game->FILEPATH, "bmps/hp_header_low.bmp") );
	game->bmp.fps_header = loadBitmap( get_filepath(game->FILEPATH, "bmps/fps_header.bmp") );
	game->bmp.teleport_ready_header = loadBitmap( get_filepath(game->FILEPATH, "bmps/jmpheaderrdy.bmp") );
	game->bmp.teleport_not_ready_header = loadBitmap( get_filepath(game->FILEPATH,"bmps/jmpheadernotrdy.bmp") );
	game->bmp.medium_score = loadBitmap( get_filepath(game->FILEPATH, "bmps/mediumscore.bmp") );
	game->bmp.large_score = loadBitmap( get_filepath(game->FILEPATH, "bmps/largescore.bmp") );
	game->bmp.alien_score = loadBitmap( get_filepath(game->FILEPATH, "bmps/alienscore.bmp") );
	game->bmp.mp_win_screen = loadBitmap( get_filepath(game->FILEPATH, "bmps/mpwinscreen.bmp") );
	game->bmp.mp_loss_screen = loadBitmap( get_filepath(game->FILEPATH, "bmps/mplossscreen.bmp") );
}

void free_bitmaps(game_data *game) {
	
	deleteBitmap(game->bmp.menu);
	deleteBitmap(game->bmp.options);
	deleteBitmap(game->bmp.boxticked);
	deleteBitmap(game->bmp.game_background);
	deleteBitmap(game->bmp.host_connecting);
	deleteBitmap(game->bmp.client_connecting);
	deleteBitmap(game->bmp.connected);
	deleteBitmap(game->bmp.pause_message);
	deleteBitmap(game->bmp.splash);
	deleteBitmap(game->bmp.death_screen);
	deleteBitmap(game->bmp.death_screen_highscore);
	deleteBitmap(game->bmp.score_header);
	deleteBitmap(game->bmp.hp_header);
	deleteBitmap(game->bmp.hp_header_low);
	deleteBitmap(game->bmp.fps_header);
	deleteBitmap(game->bmp.teleport_ready_header);
	deleteBitmap(game->bmp.teleport_not_ready_header);
	deleteBitmap(game->bmp.medium_score);
	deleteBitmap(game->bmp.large_score);
	deleteBitmap(game->bmp.alien_score);
	deleteBitmap(game->bmp.mp_win_screen);
	deleteBitmap(game->bmp.mp_loss_screen);
}

mpoint vector_translate_gfx(mpoint *vector_space, unsigned int screen_width, unsigned int screen_height) {
	mpoint gfx_point;
	mpoint origin;
	origin.x = screen_width / 2;
	origin.y = screen_height / 2;

	gfx_point.x = origin.x + vector_space->x;
	gfx_point.y = origin.y - vector_space->y;

	return gfx_point;
}



