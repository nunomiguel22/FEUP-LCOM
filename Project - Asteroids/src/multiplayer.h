#pragma once
#include <lcom/lcf.h>
#include "gamestate.h"

/** @defgroup multiplayer multiplayer
 * @{
 * Functions related to multiplayer
 */

/* Functions */

 /**
 * @brief Handles receiving serial port data
 *
 * @param COM_ADDR Address of serial port COM port
 * @param game General game struct
 */
void serial_interrupt_handler (uint16_t COM_ADDR, game_data *game);
 /**
 * @brief Handles the initial contact between computers
 *
 * @param game General game struct
 */
void mp_connect (game_data *game);
 /**
 * @brief Handles events when in multiplayer mode
 *
 * @param game General game struct
 */
void mp_playing_event_handler(game_data* game);
/**
 * @brief Initiates ships values for multiplayer mode
 *
 * @param game General game struct
 */
void mp_ship_spawn(game_data *game);
/**
 * @brief Updates the ship's force vector based on a keyboard input in multiplayer mode
 *
 * No port and starboard engines
 *
 * @param p Ship player struct
 * @param keyboard_event Keyboard input
 */
void mp_ship_apply_force (kb_evt_tp *keyboard_event, player *p);
/**
 * @brief Receives mouse input and updates the crosshair's position, checks for bounds
 *
 * @param p Ship player struct
 * @param mouse_event Mouse Input
 */
void mp_crossh_update(struct packet *mouse_event, player *p);
/**
 * @brief Handles multiplayer collision
 *
 * @param game General game struct
 */
 
bool mp_ship_collision (game_data *game);
/**
 * @brief Transmits keyboard events
 *
 * @param COM_ADDR Address of serial port COM port
 * @param game General game struct
 * @returns Returns true if a collision occurred, 0 otherwise
 */
void mp_transmit_keyboard_ev(uint16_t COM_ADDR,game_data *game);
/**
 * @brief Transmits mouse events
 *
 * @param COM_ADDR Address of serial port COM port
 * @param game General game struct
 */
void mp_transmit_mouse_ev(uint16_t COM_ADDR, game_data *game);
/**
 * @brief Transmits both players health points
 *
 * @param COM_ADDR Address of serial port COM port
 * @param game General game struct
 */
void mp_transmit_hp(uint16_t COM_ADDR, game_data *game);
/**
 * @brief Transmits connection message
 *
 * @param COM_ADDR Address of serial port COM port
 * @param game General game struct
 */
void mp_transmit_connect (uint16_t COM_ADDR);


