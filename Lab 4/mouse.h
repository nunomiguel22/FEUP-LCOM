#pragma once
#include <lcom/lcf.h>

/*
*  Mouse interrupts (un)subscription
*/

int (mouse_subscribe_int)(uint8_t *bit_no);

int (mouse_unsubscribe_int)(uint8_t *bit_no);

//verifies parity and timeout errors

int (verify_stat_error)(); 

/*
*  KBC functions to issue commands, read data/command byte, write arguments
*/

int (kbc_issue_cmd)(uint8_t cmd);//Issues a given command to the kbc

int (kbc_write_argument)(uint32_t arg); //Writes arguments to be used by the write command

int (kbc_read_return) (uint32_t *arg); //Reads a kbc command's return data

uint32_t (kbc_read_command)(); //Returns the kbc's command byte

void (default_kbc)(); //Resets the kbc's command byte to minix defaults

/*
*  KBC mouse specific functions
*/

int (enable_data_reporting) ();

int (disable_data_reporting) ();

int (set_data_reading)();

int (set_stream_mode)();

int (reset_mouse)();

int (default_mouse)();

struct packet (parse_mpacket) (uint8_t byte1, uint8_t byte2, uint8_t byte3); //parses mouse packets

/*
*  Mouse status byte functions and struct
*/

struct m_status {
	bool fixed0;
	bool mode;
	bool data_reporting;
	bool scaling;
	bool fixed00;
	bool lb;
	bool mb;
	bool rb;
	uint8_t bytes[3];
};

int (get_m_status) (); //Calls, reads, parses and prints mouse's status bytes 

struct m_status (parse_m_status) (uint8_t byte1, uint8_t byte2, uint8_t byte3); //Parses mouse's status byte

void (print_m_status) (struct m_status *ms); //Prints mouse's status bytes


/*
*  State machine functions and enum
*/

typedef enum {INIT, DRAWLINE1, VERTEX, DRAWLINE2, COMP} state_t; 

int (state_machine) (struct mouse_ev *ms, state_t *ges, uint8_t x_len, uint8_t tolerance);

struct mouse_ev (m_detect_evt) (struct packet *pp);

