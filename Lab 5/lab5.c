// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>
#include <stdint.h>
#include <stdio.h>
#include <liblm.h>

// Any header files included below this line should have been created by you
#include "vcard.h"
#include "macros.h"


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int (video_test_init)(uint16_t mode, uint8_t delay) {
	
	vg_init(mode);

	/*Tried using timer 0 but tests failed with message "one or more missing/unexpected function calls"
	* eventhough it worked apparently. Test passes with sleep function.
	*/
	
	//exit_gfx_on_delay(delay); //uncomment this for delay using timer 0
	
	sleep(delay); 
	
	vg_exit();
	
  return 0;
}

int (video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height, uint32_t color) {

	
	vg_init(mode);

	draw_rectangle(x, y, width, height, color);
	
	exit_gfx_on_esc();
	

  return 0;
}

int (video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
	

	vbe_mode_info_t mode_info;
	vbe_info(mode, &mode_info);
	
	unsigned int pixel_bits = mode_info.BitsPerPixel; 
	
	//Calculate rectangle size
	unsigned int cols = mode_info.XResolution / no_rectangles;
	unsigned int rows = mode_info.YResolution / no_rectangles;
	
	
	vg_init(mode);

	//Index mode
	if (pixel_bits == 8) {
		for (int i = 0; i < no_rectangles; i++)
			for (int j = 0; j < no_rectangles; j++){
				uint8_t color = (first + ((j * no_rectangles) + i) * step) % (1 << pixel_bits);
				draw_rectangle(i * cols , j * rows, cols, rows, color);
			}
	}
	
     //Direct mode

	if (pixel_bits == 15){
        uint8_t red_first = (first >> 10) , green_first = (first >> 5), blue_first = first;
		
		blue_first &= 0x1F;
        green_first &= 0x1F;
        red_first &= 0x1F;
		
        for (int i = 0; i < no_rectangles; i++)
			for (int j = 0; j < no_rectangles; j++){
                uint32_t color;            
				color = (red_first + i * step) % (1 << mode_info.RedMaskSize);
				color = color << 5;
				color += (green_first + j * step) % (1 << mode_info.GreenMaskSize);
				color = color << 5;
				color += (blue_first + (i + j) * step) % (1 << mode_info.BlueMaskSize);
				draw_rectangle(i * cols , j * rows, cols, rows, color);
			}
	}
                
	if (pixel_bits == 16){
		
        uint8_t red_first = (first >> 11) , green_first = (first >> 5), blue_first = first;
		blue_first &= 0x1F;
		green_first &= 0x3F;
		red_first &= 0x1F;
                
		for (int i = 0; i < no_rectangles; i++)
			for (int j = 0; j < no_rectangles; j++){
				uint32_t color;
				color = (red_first + i * step) % (1 << mode_info.RedMaskSize);
   	            color = color << 5;
                color += (green_first + j * step) % (1 << mode_info.GreenMaskSize);
				color = color << 6;
				color += (blue_first + (i + j) * step) % (1 << mode_info.BlueMaskSize);
				draw_rectangle(i * cols , j * rows, cols, rows, color);
			}
	}
        
        
	if (pixel_bits == 24){
		
		uint8_t red_first = (first >> 16) , green_first = (first >> 8), blue_first = first;        
                
		for (int i = 0; i < no_rectangles; i++)
			for (int j = 0; j < no_rectangles; j++){
				uint32_t color;
				color = (red_first + i * step) % (1 << mode_info.RedMaskSize);
				color = color << 8;
				color += (green_first + j * step) % (1 << mode_info.GreenMaskSize);
				color = color << 8;
				color += (blue_first + (i + j) * step) % (1 << mode_info.BlueMaskSize);
				draw_rectangle(i * cols , j * rows, cols, rows, color);
			}
	}
        
	
	exit_gfx_on_esc();
	
	return 0;
	
}

int (video_test_xpm) (const char *xpm[], uint16_t x, uint16_t y){
	
	vg_init(indexed1024);
	
	draw_xpm(xpm, x, y);
	
	exit_gfx_on_esc();
	
	
	return 0;
}


int (video_test_move) (const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate){
	
	vg_init(indexed1024);
	
	if (speed >= 0) 
		move_xpm_frame(xpm, xi, yi, xf, yf, speed, fr_rate);
	else move_xpm_pixel(xpm, xi, yi, xf, yf, (speed * -1), fr_rate);
	exit_gfx_on_esc();
	
	
return 0;	
}


int (video_test_controller)()
{

	
	vg_vbe_contr_info_t info_p;
	
	vbe_getControlInfo(&info_p);
	
	
	//vg_display_vbe_contr_info(&info_p);
	
	
	return 0;
}



