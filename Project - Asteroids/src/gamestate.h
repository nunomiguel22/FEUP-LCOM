#pragma once
#include <lcom/lcf.h>
#include "mvector.h"
#include "macros.h"
#include "graphics.h"
#include "devices.h"
#include "ship.h"
#include "asteroidfield.h"
#include "serialport.h"

/** @defgroup gamestate gamestate
 * @{
 * Functions related to game logic
 */
 
 /* Data structures */
 
 /** @brief Enum with all gamestates */
typedef enum {MENU, OPTIONSMENU, START_SEQUENCE, PLAYING, NEW_ROUND, LOSS, GAMEPAUSED, CONNECTING, MULTIPLAYER, MULTIPLAYEROVER, COMP} game_sts;

 /** @brief Saves the menu's settings */
typedef struct {
	int fps;										/**< @brief Frames per second */
	float m_sens;									/**< @brief Mouse sensitivity */
	bool page_flip;									/**< @brief Page flipping */
	bool vsync;										/**< @brief Vertical sync */
	bool fps_counter;								/**< @brief In game frame counter */
}menu_options;

/** @brief Main game data structure */
typedef struct {
	game_sts state;									/**< @brief Current game state */
	game_timers timers;								/**< @brief Struct with the game's timers */
	player player1;									/**< @brief Player1, this is always the ship controlled by this computer */
	player player2;									/**< @brief Player2, this is the enemy ship in multiplayer mode */
	player alien;									
	asteroid asteroid_field[MAX_ASTEROIDS];			/**< @brief Asteroid array */
	unsigned int highscores[5];						/**< @brief Highscores array, up to 5 highscores */
	
	mpoint cursor;									/**< @brief Menus cursor location */
	event_type event;								/**< @brief Type of input event */
	struct packet mouse_event;						/**< @brief Player1 mouse event */
	kb_evt_tp keyboard_event;						/**< @brief Player1 keyboard event */
	serial_evt_tp serial_event;						/**< @brief Serial port event, message type */
	
	struct packet p2_mouse_event;					/**< @brief Player2 mouse event */
	kb_evt_tp p2_keyboard_event;					/**< @brief Player2 keyboard event */
	bool host;										/**< @brief When true player1 is host in multiplayer */
	bool mpconnection;								/**< @brief When true a multiplayer game is ongoing */
	
	char *gr_buffer;								/**< @brief Pointer to local pixel buffer */
	char FILEPATH[50];								/**< @brief Filepath to game folder */
	bitmap_data bmp;								/**< @brief All bmps images */
	pixmap_data xpm;								/**< @brief All pixmaps */
	menu_options settings;							/**< @brief Options menu's settings */
	rtc_time_t time_of_day;							/**< @brief RTC time */
	
} game_data;

 /* Functions */

 /**
 * @brief Updates cursor within screen bounds
 *
 * @param game General game struct
 */
void cursor_update (game_data *game);
 /**
 * @brief This function does all singleplayer physics operations
 *
 * @param game General game struct
 */
void physics_update (game_data *game);
 /**
 * @brief This function does all multiplayer physics operations
 *
 * @param game General game struct
 */
void mp_physics_update (game_data *game);
 /**
 * @brief Renders singleplayer frames, allows for page flipping and vsync. Clears the local pixel buffer 
 *
 * @param game General game struct
 */
void handle_frame (game_data * game);
 /**
 * @brief Renders multiplayer frames, allows for page flipping and vsync. Clears the local pixel buffer 
 *
 * @param game General game struct
 */
void mp_handle_frame (game_data * game);
 /**
 * @brief Renders menu frames, allows for page flipping and vsync. Clears the local pixel buffer 
 *
 * @param game General game struct
 * @param bckgrd Menu bitmap
 */
void handle_menu_frame (game_data *game, Bitmap *bckgrd);
 /**
 * @brief Initiates/resets all game timers
 *
 * @param timers Game timers
 */
void start_timers (game_timers *timers);
 /**
 * @brief Initiates/resets all game data
 *
 * @param game General game struct
 */
void game_data_init(game_data *game);
 /**
 * @brief Main loop, handles all interrupt events
 *
 * @param game General game struct
 */
void interrupt_handler(game_data* game);
 /**
 * @brief Main game state machine
 *
 * @param game General game struct
 */
void game_state_machine (game_data* game);
 /**
 * @brief Handles events when in singleplayer mode
 *
 * @param game General game struct
 */
void playing_event_handler(game_data* game);


