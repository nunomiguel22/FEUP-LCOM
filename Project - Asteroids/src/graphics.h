#pragma once
#include <lcom/lcf.h>

/** @defgroup graphics graphics
 * @{
 * Functions related to Bitmaps and Pixmaps
 */

/* Auxiliary functions */ 

/**
 * @brief Concatenates a folder path with file/folder name
 *
 * @param folder Path to game folder
 * @param folder Path from game folder to desired file
 * @return Returns the full path to the desired file
 */
char * get_filepath (char * folder, char * file);

/*
Pixmap-related functions
*/

/**
 * @brief Pixmap struct
 */
typedef struct{
	uint32_t *map;					/**< @brief Pixmap data */
	int height;						/**< @brief Pixmap height */
	int width;						/**< @brief Pixmap width */
}pixmap;

/**
 * @brief Struct will all pixmaps used in the game
 */
typedef struct{
	pixmap ship_blue;
	pixmap ship_blue_bt;
	pixmap ship_blue_pt;
	pixmap ship_blue_st;
	
	pixmap asteroid_large;
	pixmap asteroid_medium;
	pixmap asteroid_dest1;
	pixmap asteroid_dest2;
	pixmap asteroid_dest3;
	
	pixmap ship_red;
	pixmap ship_red_bt;
	pixmap ship_red_pt;
	pixmap ship_red_st;
	
	pixmap alien_ship;
	
	pixmap cursor;
	pixmap crosshair;
	pixmap blue_laser;
	pixmap red_laser;

	pixmap n_colon;
	pixmap n_zero;
	pixmap n_one;
	pixmap n_two;
	pixmap n_three;
	pixmap n_four;
	pixmap n_five;
	pixmap n_six;
	pixmap n_seven;
	pixmap n_eight;
	pixmap n_nine;
	pixmap n_one_large;
	pixmap n_two_large;
	pixmap n_three_large;

}pixmap_data;

/**
 * @brief Loads pixmaps with 24 bit direct color mode
 *
 * Initially based on Joao Cardoso's "read_xpm()" and later heavily modified
 *
 * @param map Pixmap to be loaded
 * @param wd Pointer to variable where the pixmap's width will be saved
 * @param ht Pointer to variable where the pixmap's height will be saved
 * @return Returns a pointer to the parsed and loaded pixmap
 */
uint32_t* read_pixmap24 (const char *map[], int *wd, int *ht);
/**
 * @brief Draws pixmaps to a buffer, allows for transparency and rotation
 *
 * All pixmaps are assumed to be draw at a 90º angle by default
 *
 * @param xpm Pixmap to be drawn
 * @param gx Graphical x coordinate where pixmap will be drawn
 * @param gy Graphical y coordinate where pixmap will be drawn
 * @param degrees Pixmap rotation in degrees
 */
void draw_pixmap (pixmap *xpm, uint16_t gx, uint16_t gy, double rotation_degrees);

/*
Bitmap-related functions
*/


typedef enum { ALIGN_LEFT, ALIGN_CENTRE, ALIGN_RIGHT } Alignment;

typedef struct {
	unsigned short type; 	//specifies file type
	unsigned int size; 		//size (in bytes9 of file
	unsigned int reserved; 	//reserved, must be 0
	unsigned int offset; 	//specifies offset (in bytes) of the bitmapFileHeader to the bitmap's bits
} BitmapFileHeader;

typedef struct {
	
	unsigned int size; 				//specifies no. of bytes required by the struct
	int width; 						//width (in px)
	int height; 					//height (in px)
	unsigned short planes;			//specifies no. of colour planes, must be 1
	unsigned short bits; 			//no. of bits per pixel
	unsigned int compression; 		//type of compression
	unsigned int imageSize; 		//size of image in bytes
	int xResolution; 				//pixels per meter in X-axis
	int YResolution; 				//pixels per meter in Y-axis
	unsigned int nColours; 			//number of colours used by the bitmap
	unsigned int importantColours; 	//number of important colours in the bitmap
} BitmapInfoHeader;

typedef struct
{
	BitmapInfoHeader bitmapInfoHeader;
	unsigned char * bitmapData;
} Bitmap;

typedef struct{
	Bitmap *menu;
	Bitmap *options;
	Bitmap *boxticked;
	Bitmap *game_background;
	Bitmap *host_connecting;
	Bitmap *client_connecting;
	Bitmap *connected;
	Bitmap *pause_message;
	Bitmap *splash;
	Bitmap *death_screen;
	Bitmap *death_screen_highscore;
	Bitmap *score_header;
	Bitmap *hp_header;
	Bitmap *hp_header_low;
	Bitmap *fps_header;
	Bitmap *teleport_ready_header;
	Bitmap *teleport_not_ready_header;
	Bitmap *medium_score;
	Bitmap *large_score;
	Bitmap *alien_score;
	Bitmap *mp_loss_screen;
	Bitmap *mp_win_screen;
}bitmap_data;

/**
 * @brief Loads a bmp image, done by Henrique Ferrolho
 *
 * @param filename Path to the bmp
 * @return NULL on error, pointer to loaded bitmap otherwise
 * @see http://difusal.blogspot.pt/2014/09/minixtutorial-8-loading-bmp-images.html
 */
Bitmap* loadBitmap (const char* filename);

/**
 * @brief Draws an unscaled, unrotated bitmap at the given position, done by Henrique Ferrolho
 *
 * Altered to allow for transparency
 *
 * @param bmp bitmap to be drawn
 * @param x destiny x coord
 * @param y destiny y coord
 * @see http://difusal.blogspot.pt/2014/09/minixtutorial-8-loading-bmp-images.html
 */
void drawBitmap(Bitmap* bitmap, int x, int y);

/**
 * @brief Destroys the given bitmap, freeing all resources used by it, done by Henrique Ferrolho
 *
 * @param bitmap bitmap to be destroyed
 * @see http://difusal.blogspot.pt/2014/09/minixtutorial-8-loading-bmp-images.html
 */
void deleteBitmap(Bitmap* bmp);

