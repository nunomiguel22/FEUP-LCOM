#pragma once
#include <lcom/lcf.h>

/** @defgroup serialport serialport
* @{
*	All code relating to the serial port
*/

 /* Data structures */
 
/** @brief Serial port message types */
typedef enum {NEWTRNM, CONNECTED, KB, MS, HP, SC, C_MSG} serial_evt_tp;

/**
 * @brief Gets COM1 line control register
 *
 * @returns Returns COM1 line control register
 */
uint8_t sp_control ();
/**
 * @brief Gets COM1 line status register
 *
 * @returns Returns COM1 line status register
 */
uint8_t sp_status ();
/**
 * @brief Writes character to transmitter holding register
 *
 * @param COM_ADDR Address of serial port COM port
 * @param cdata Character to write
 */
void sp_write_th (uint16_t COM_ADDR, uint32_t cdata);
/**
 * @brief Reads character from receiver buffer register
 *
 * @param COM_ADDR Address of serial port COM port
 * @param cdata pointer to where character will be stored
 */
void sp_read_buffer (uint16_t COM_ADDR, uint8_t *cdata);
/**
 * @brief Subscribes to serial port interrupts
 */
int sp_subscribe_int ();
/**
 * @brief Unubscribes to serial port interrupts
 */
int sp_unsubscribe_int();
/**
 * @brief Enables serial port interrupts
 */
void sp_enable_int();
/**
 * @brief Disables serial port interrupts
 */
void sp_disable_int();
/**
 * @brief Status of transmitter holding register
 *
 * @returns True if the transmitter holding register is ready, 0 otherwise
 */
bool sp_transmitter_ready(uint16_t COM_ADDR);
/**
 * @brief Status of receiver buffer register
 *
 * @returns True if the receiver buffer register is ready, 0 otherwise
 */
bool sp_data_available(uint16_t COM_ADDR);
/**
 * @brief Sets serial port bit rate
 *
 * @param COM_ADDR Address of serial port COM port
 * @param rate Bit rate
 */
void sp_set_rate(uint16_t COM_ADDR, uint32_t rate);
/**
 * @brief Prints serial ports line control register
 *
 */
void sp_print_config ();
/**
 * @brief Initializes serial port's fifo buffer
 *
 * No parity checking, interrupts on every byte
 *
 * @param COM_ADDR Address of serial port COM port
 */
void sp_initialize_fifo(uint16_t COM_ADDR);


