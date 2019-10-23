#pragma once
#include <lcom/lcf.h>


//Interrupt subscription
int (keyboard_subscribe_int)(uint8_t *bit_no);

int (keyboard_unsubscribe_int)(uint8_t *bit_no);

//sys_inb wrapper with counter
int (sys_inb_cnt)(port_t port, uint32_t *byte); 


//Lower level of kbc functions
int (kbc_issue_cmd)(uint8_t cmd);//Issues a given command to the kbc

uint32_t (kbc_read_return) (); //Reads returns from kbc commands - NOT WORKING

void (kbc_write_argument)(uint32_t arg); //Writes arguments to be used by the write command

uint32_t (kbc_read_status)(); //Returns the status byte 

//Higher Level of kbc funtions

void (kbc_write_command)(uint32_t arg); //Issues the write command and writes the command byte to the status reg

uint32_t (kbc_read_command)();//reads returns from kbc commands - WORKING

uint32_t (kbc_self_test)(); //Performs kbc self test, returns "55" if OK

uint32_t (kbc_check_keyboard_interface)();//Checks the status of the keyboard interface, "0" if enabled.

void (kbc_toggle_keyboard_interrupts) ();//Toggles the keyboard's interrupts;

int (verify_stat_error)(); //verifies parity and timeout errors


