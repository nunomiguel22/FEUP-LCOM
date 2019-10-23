#pragma once

static uint16_t hres;
static uint16_t vres;
static unsigned int pixel_bits;
static uint8_t *gcard_v_address;
extern uint32_t code;
extern unsigned int timerTickCounter;

int (timer_unsub_int)(uint8_t *bit_no); 

int draw_pixel(unsigned int x, unsigned int y, unsigned int color);
int draw_rectangle (unsigned int x, unsigned int y, unsigned int width, unsigned int height, uint32_t color);
int draw_xpm (const char *xpm[], uint16_t x, uint16_t y);
int remove_xpm (const char *xpm[], uint16_t x, uint16_t y);
int move_xpm_pixel (const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate);
int move_xpm_frame (const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate);
int vbe_info (uint16_t mode, vbe_mode_info_t *mode_info);
int vbe_getControlInfo(vg_vbe_contr_info_t * info_p);

void exit_gfx_on_esc();
void exit_gfx_on_delay(uint8_t delay);


