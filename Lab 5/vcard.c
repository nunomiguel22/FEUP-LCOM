#include <lcom/lcf.h>
#include <liblm.h>
#include "macros.h"
#include "vcard.h"
#include "keyboard.h"
#include <stdio.h>

void* (vg_init) (uint16_t mode) {
	
	
	void *video_mem;
	vbe_mode_info_t mode_info;
	struct reg86u rg;
	unsigned int vram_base;
	unsigned int vram_size;
	struct minix_mem_range mr;
	int r;
	
	lm_init(false);
	vbe_info(mode, &mode_info);
	hres = mode_info.XResolution;
	vres = mode_info.YResolution;
	pixel_bits = mode_info.BitsPerPixel; 
	
	
	memset(&rg, 0, sizeof(rg));
	
	mode =  0x1<<14|mode;
	
	rg.u.w.ax = set_vbe_mode;
	rg.u.w.bx = mode;
	rg.u.b.intno = video_card_bios;


	
	
	
	vram_base = mode_info.PhysBasePtr;
	vram_size = hres * vres * (pixel_bits / 8);
	
	/* Allow memory mapping */
	
	mr.mr_base = (phys_bytes) vram_base;
	mr.mr_limit = mr.mr_base + vram_size;
	
	if ( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		panic("sys_privctl (ADD_MEM) failed: %d\n", r);
	
	/* Map memory */
	
	video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
	gcard_v_address = video_mem;
	
	if (video_mem == MAP_FAILED)
		panic ("couldn't map video memory");
	
	if( sys_int86(&rg) != OK )
		printf ("Failed to set mode\n");
	
	return video_mem;
		
}

int vbe_info (uint16_t mode, vbe_mode_info_t *mode_info){
	
	mmap_t map;
	
	struct reg86u rg;
	memset(&rg, 0, sizeof(rg));
	
	lm_init(false);
	
	lm_alloc(256, &map);
	
	rg.u.w.ax = get_vbe_info;
	rg.u.w.es = PB2BASE (map.phys);
	rg.u.w.di = PB2OFF (map.phys);
	rg.u.w.cx = mode;
	rg.u.b.intno = video_card_bios;
	
	if( sys_int86(&rg) != OK )
		return -1;
	
	memcpy(mode_info,map.virt, 256);
	
	lm_free (&map);
	
	return 0;
	
}

int draw_pixel(unsigned int x, unsigned int y, uint32_t color){
	
	uint8_t *v_address = gcard_v_address;
	
	if (x > hres || y > vres){
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

int draw_rectangle (unsigned int x, unsigned int y, unsigned int width, unsigned int height, uint32_t color){
	
	for (unsigned int i = 0; i < width; i++){
		for (unsigned int j = 0; j < height; j++){
			if (draw_pixel(x + i, y + j, color) )
				return 1;
		}
	}
	return 0;	
}

int draw_xpm (const char *xpm[], uint16_t x, uint16_t y){
	int width, height;
	char *map = read_xpm(xpm, &width, &height);
		
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++){
			uint32_t color = *(map + (i * width) + j);
			draw_pixel(x + j, y + i, color);
		}
	
	free(map);
	
	return 0;
}

int remove_xpm (const char *xpm[], uint16_t x, uint16_t y){
	int width, height;
	char *map = read_xpm(xpm, &width, &height);
		
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++){
			draw_pixel(x + j, y + i, 0);
		}
	
	free(map);
	
	return 0;
}

//Move one pixel per a number of frames
int move_xpm_pixel (const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate){
	
	uint16_t x = xi, y = yi;
	int frames = 60 / fr_rate;
	int inthcounter = 0;
	int framecounter = 0;
	
	//Keyboard subscription
	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	keyboard_subscribe_int (hook_id);
	
	//Timer subscription
	uint8_t *timer_hook_id; 
	timer_hook_id = (uint8_t*) malloc (1);
	timer_subscribe_int (timer_hook_id); 
	
	int ipc_status;
	message msg;
	
	//Initial position
	draw_xpm (xpm, x, y);
	
	while(x != xf || y != yf)  {
	
		int r = driver_receive(ANY, &msg, &ipc_status); 
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { 
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: 
					if (msg.m_notify.interrupts & irq_timer0) {
						inthcounter++;
						if (inthcounter % frames == 0)
							framecounter++;
							
							
						if (framecounter == speed){
							
							remove_xpm(xpm, x, y);		
							
							//Vertical axis
							if (xi == xf){
								if (yf > yi)
								y++;
								else y--;	
								draw_xpm(xpm, x, y);
								framecounter = 0;					
							}
						
							//Horizontal axis
							if (yi == yf){
								if (xf > xi)
								x++;
								else x--;
								draw_xpm(xpm, x, y);
								framecounter = 0;
							}				
						}
					}
					
					if (msg.m_notify.interrupts & irq_keyboard) {
						
						
						//if(verify_stat_error() == 0){
							sys_inb(OUT_BUF, &code);
							if (code == 0x81){
								keyboard_unsubscribe_int(hook_id);
								timer_unsub_int(timer_hook_id);	
								timer_set_frequency(0, 60);
								vg_exit();
								return 0;
								
							}
								
						//	}
					}
					
                break;
				default:
					break; 
			}
		}
	}
	
	
	
	keyboard_unsubscribe_int(hook_id);
	timer_unsub_int(timer_hook_id);	
	timer_set_frequency(0, 60);
	
	return 0;
	
}

//Move a number of pixels per frame
int move_xpm_frame (const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate){
	
	int frames = 60 / fr_rate;
	int framecounter = 0;
	uint16_t x = xi, y = yi;
	
	//Keyboard subscription
	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	keyboard_subscribe_int (hook_id);
	
	//Timer subscription
	uint8_t *timer_hook_id;
	timer_hook_id = (uint8_t*) malloc (1);
	
	int ipc_status;
	message msg;
	
	draw_xpm (xpm, x, y);
	
	timer_subscribe_int (timer_hook_id); 
	
	while (x != xf || y != yf) {
	
		int r = driver_receive(ANY, &msg, &ipc_status); 
		if ( r != 0 ) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { 
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: 
					if (msg.m_notify.interrupts & irq_timer0) {	
						framecounter++;
						if (framecounter == frames){
							framecounter = 0;
							remove_xpm(xpm, x, y);		
							
							//Vertical axis
							if (xi == xf){
								if (yf > yi)
								y += speed;
								else y -= speed;	
								draw_xpm(xpm, x, y);
								framecounter = 0;					
							}
						
							//Horizontal axis
							if (yi == yf){
								if (xf > xi){
									if (x > xf)
										x = xf;
									else x += speed;
								}	
							
								else {
									if (xf > x)
										x = xf;
									else x -= speed;
							
									 }
							
								draw_xpm(xpm, x, y);
								framecounter = 0;
							
							}
						}
					}
					
					if (msg.m_notify.interrupts & irq_keyboard) {
						
						
						//if(verify_stat_error() == 0){
							sys_inb(OUT_BUF, &code);
							if (code == 0x81){
								keyboard_unsubscribe_int(hook_id);
								timer_unsub_int(timer_hook_id);	
								timer_set_frequency(0, 60);
								vg_exit();
								return 0;
								
							}
								
						//	}
					}
					
                break;
				default:
					break; 
			}
		}
	}
	
	keyboard_unsubscribe_int(hook_id);
	timer_unsub_int(timer_hook_id);	
	timer_set_frequency(0, 60);
	
	return 0;
	
}

void exit_gfx_on_delay(uint8_t delay){
		
	uint8_t *timer_hook_id; 
	timer_hook_id = (uint8_t*) malloc (1);
	int ipc_status;
	message msg;
	
	timer_subscribe_int (timer_hook_id); 
	
	while(timerTickCounter / 60 < delay)  {
	
		int r = driver_receive(ANY, &msg, &ipc_status); 
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { 
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: 
					if (msg.m_notify.interrupts & irq_timer0) {
						timer_int_handler();
					}
                break;
				default:
					break; 
			}
		}
	}
	timer_unsub_int(timer_hook_id);
	
	vg_exit();

	
}

void exit_gfx_on_esc(){
	
	//Subscribes to keyboard interupts using hook_id
	uint8_t *hook_id;
	hook_id = (uint8_t*) malloc (1);
	keyboard_subscribe_int (hook_id);
	
	int ipc_status;
	message msg;

	while(code != 0x81)  {
		
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) { 
			printf("driver_receive failed with: %d", r);
			continue;
		}
		
		if (is_ipc_notify(ipc_status)) { 
		
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
						
					if (msg.m_notify.interrupts & irq_keyboard) {
						
						
						//if(verify_stat_error() == 0){
							sys_inb(OUT_BUF, &code);
						//	}
					}
                break;
				default:
					break; 
			}
		}
	}
	
	//Unsubscribes from the keyboard interrupts and returns to text mode
	keyboard_unsubscribe_int(hook_id);
	vg_exit();
}

int vbe_getControlInfo(vg_vbe_contr_info_t * info_p)
{
	//initialize vars
	struct reg86u rg;
	memset(&rg, 0, sizeof(rg));
	mmap_t map;
	
	void *mem = lm_init(false);
	printf("%X\n", mem);
	
	lm_alloc(512, &map);
	
	//Set VBE 2.0
	
	info_p->VBESignature[0] = 'V';
	info_p->VBESignature[1] = 'B';
	info_p->VBESignature[2] = 'E';
	info_p->VBESignature[3] = '2';
	
	memcpy(map.virt,info_p, sizeof(*info_p));
	
	//Get info
	
	rg.u.w.es = PB2BASE (map.phys);
	rg.u.w.di = PB2OFF (map.phys);
	rg.u.b.intno = video_card_bios;
	rg.u.w.ax = 0x4F00;
	
	if( sys_int86(&rg) != OK )
		return -1;
	
	memcpy(info_p, map.virt, sizeof(*info_p));
	
	//Incomplete
	
	lm_free(&map);
	
	return 0;
}




