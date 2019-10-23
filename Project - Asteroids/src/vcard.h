#pragma once
#include <lcom/lcf.h>
#include "mvector.h"
#include "ship.h"
#include "asteroidfield.h"
#include "gamestate.h"

/** @defgroup vcard vcard
 * @{
 * Functions related to the video card and frame rendering
 */
 
  /* Data structures */

static unsigned int pixel_bits;				/**< @brief Number of bits per pixel */
static uint8_t *gcard_v_address;			/**< @brief Vram virtual address */
static phys_bytes gcard_phys_address;		/**< @brief Vram physical address, used for page flipping */
static uint8_t *pixel_buffer;				/**< @brief Local pixel buffer address */
static unsigned int vram_size;				/**< @brief Size of allocated vram */

#pragma pack(1)
/** @brief VBE mode info block */
typedef struct {

  uint16_t ModeAttributes; 					/**< @brief mode attributes */
  uint8_t WinAAttributes; 					/**< @brief window A attributes */
  uint8_t WinBAttributes; 					/**< @brief window B attributes */
  uint16_t WinGranularity; 					/**< @brief window granularity */
  uint16_t WinSize;							/**< @brief window size */
  uint16_t WinASegment;						/**< @brief window A start segment */
  uint16_t WinBSegment;						/**< @brief window B start segment */
  phys_bytes WinFuncPtr;					/**< @brief real mode/far pointer to window function */
  uint16_t BytesPerScanLine; 				/**< @brief bytes per scan line */
  uint16_t XResolution;      				/**< @brief horizontal resolution in pixels/characters */
  uint16_t YResolution;      				/**< @brief vertical resolution in pixels/characters */
  uint8_t XCharSize; 						/**< @brief character cell width in pixels */	
  uint8_t YCharSize; 						/**< @brief character cell height in pixels */
  uint8_t NumberOfPlanes; 					/**< @brief number of memory planes */	
  uint8_t BitsPerPixel; 					/**< @brief bits per pixel */
  uint8_t NumberOfBanks;					/**< @brief number of banks */
  uint8_t MemoryModel;						/**< @brief memory model type */
  uint8_t BankSize;							/**< @brief bank size in KB */
  uint8_t NumberOfImagePages;				/**< @brief number of images */
  uint8_t Reserved1;		 				/**< @brief reserved for page function */
  uint8_t RedMaskSize;						/**< @brief size of direct color red mask in bits */
  uint8_t RedFieldPosition;					/**< @brief bit position of lsb of red mask */
  uint8_t GreenMaskSize;					/**< @brief size of direct color green mask in bits */
  uint8_t GreenFieldPosition;				/**< @brief bit position of lsb of green mask */
  uint8_t BlueMaskSize; 					/**< @brief size of direct color blue mask in bits */
  uint8_t BlueFieldPosition;				/**< @brief bit position of lsb of blue mask */
  uint8_t RsvdMaskSize;						/**< @brief size of direct color reserved mask in bits */
  uint8_t RsvdFieldPosition;				/**< @brief bit position of lsb of reserved mask */
  uint8_t DirectColorModeInfo;				/**< @brief direct color mode attributes */

  /* VBE 2.0 and above */
	
  phys_bytes PhysBasePtr;    				/**< @brief physical address for flat memory frame buffer */
  uint8_t Reserved2[4]; 					/**< @brief Reserved - always set to 0 */	
  uint8_t Reserved3[2]; 					/**< @brief Reserved - always set to 0 */	

  /* VBE 3.0 and above */
  uint16_t LinBytesPerScanLine;   			/**< @brief bytes per scan line for linear modes */
  uint8_t BnkNumberOfImagePages; 			/**< @brief number of images for banked modes */
  uint8_t LinNumberOfImagePages; 			/**< @brief number of images for linear modes */
  uint8_t LinRedMaskSize; 	     			/**< @brief size of direct color red mask (linear modes) */
  uint8_t LinRedFieldPosition; 				/**< @brief bit position of lsb of red mask (linear modes) */
  uint8_t LinGreenMaskSize; 				/**< @brief size of direct color green mask (linear modes) */
  uint8_t LinGreenFieldPosition; 			/**< @brief bit position of lsb of green mask (linear  modes) */
  uint8_t LinBlueMaskSize; 					/**< @brief size of direct color blue mask (linear modes) */
  uint8_t LinBlueFieldPosition; 			/**< @brief bit position of lsb of blue mask (linear modes ) */
  uint8_t LinRsvdMaskSize; 					/**< @brief size of direct color reserved mask (linear modes) */
  uint8_t LinRsvdFieldPosition;	 			/**< @brief bit position of lsb of reserved mask (linear modes) */
  uint32_t MaxPixelClock; 	         		/**< @brief maximum pixel clock (in Hz) for graphics mode */
  uint8_t Reserved4[190]; 					/**< @brief remainder of ModeInfoBlock */
} vbe_ModeInfoBlock;
#pragma options align = reset

 /* Functions */

 /**
 * @brief Returns VBE mode information
 *
 * @param mode Graphics mode
 * @param mode_info VBE mode info struct
 * @returns Returns 0 on success, 1 otherwise
 */
int vbe_mode_info (uint16_t mode, vbe_ModeInfoBlock *mode_info);
 /**
 * @brief Switches VBE graphics mode
 *
 * @param mode Graphics mode
 * @returns Returns pointer to allocated vram virtual address
 */
void* vmode_init (uint16_t mode);
 /**
 * @brief Switches to default minix text mode
 *
 * @returns Returns 0 on success, 1 otherwise
 */
int vmode_exit();
 /**
 * @brief Switches VBE display start pointer
 *
 * @param flip If true switches pointer to first page, on false switches to second page
 * @param vsync If true switches only when a vertical retrace is ready
 * @returns Returns 0 on success, 1 otherwise
 */
int vbe_switch_display_start(bool flip, bool vsync);
 /**
 * @brief Draws a pixel in the local buffer
 *
 * @param x Horizontal location of the pixel in graphic coordinates
 * @param y Vertical location of the pixel in graphic coordinates
 * @param color Color of the pixel
 * @returns Returns 0 on success, 1 otherwise
 */
int draw_pixel(int x, int y, unsigned int color);
 /**
 * @brief Draws a pixmap digit
 *
 * @param num Digit to draw
 * @param x Horizontal location of the digit in graphic coordinates
 * @param y Vertical location of the digit in graphic coordinates
 * @param game General game struct
 */
void draw_digit(unsigned int num, int x, int y, game_data *game);
 /**
 * @brief Draws a large pixmap digit
 *
 * @param num Digit to draw
 * @param x Horizontal location of the digit in graphic coordinates
 * @param y Vertical location of the digit in graphic coordinates
 * @param game General game struct
 */
void draw_large_digit(unsigned int num, int x, int y, game_data *game);
 /**
 * @brief Draws a positive integer number on screen
 *
 * @param number Number to draw
 * @param x Horizontal location of the number in graphic coordinates
 * @param y Vertical location of the number in graphic coordinates
 * @param game General game struct
 */
void draw_number (int number, int x, int y, game_data *game);
 /**
 * @brief Draws the RTC time on screen
 *
 * @param game General game struct
 * @param x Horizontal location of the digit in graphic coordinates
 * @param y Vertical location of the digit in graphic coordinates
 */
void draw_time_of_day (game_data *game, int x, int y);
 /**
 * @brief Draws a game ship on screen
 *
 * @param xpm Ship pixmap
 * @param p Player struct
 */
void draw_ship(pixmap *xpm, player *p);
 /**
 * @brief Draws the enemy alien ship
 *
 * @param game General game struct
 */
void draw_alien(game_data *game);
 /**
 * @brief Draws an individual asteroid
 *
 * @param ast Asteroid
 * @param xpm asteroid pixmap
 */
void draw_ast(asteroid *ast, pixmap *xpm);
 /**
 * @brief Renders a singleplayer frame to the local buffer
 *
 * @param game General game struct
 */
void render_frame(game_data *game);
 /**
 * @brief Renders a multiplayer frame to the local buffer
 *
 * @param game General game struct
 */
int mp_render_frame(game_data *game);
 /**
 * @brief Renders a start sequence frame to the local buffer
 *
 * @param game General game struct
 */
void render_seq_frame(game_data *game);
 /**
 * @brief Draws the opening splash screen
 */
void show_splash();
 /**
 * @brief Displays a local buffer frame on screen
 *
 * @param page_flip If true page flipiing is used
 * @param vsync If true page flipping and vsync are used
 */
void display_frame(bool page_flip, bool vsync);
 /**
 * @brief Loads all pixmaps into memory
 *
 * @param game General game struct
 */
void load_xpms(game_data *game);
 /**
 * @brief Frees all pixmaps from memory
 *
 * @param game General game struct
 */
void free_xpms(game_data *game);
 /**
 * @brief Loads all bitmaps into memory
 *
 * @param game General game struct
 */
void load_bitmaps(game_data *game);
 /**
 * @brief Frees all bitmaps from memory
 *
 * @param game General game struct
 */
void free_bitmaps(game_data *game);
 /**
 * @brief Translates a cartesian point into graphical coordinates
 *
 * @param vector_space cartesian point
 * @param screen_width Number of horizontal pixels
 * @param y screen_height Number of vertical pixels
 */
mpoint vector_translate_gfx(mpoint *vector_space, unsigned int screen_width, unsigned int screen_height);

