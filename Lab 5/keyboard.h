#pragma once
#include <lcom/lcf.h>


//Interrupt subscription
int (keyboard_subscribe_int)(uint8_t *bit_no);

int (keyboard_unsubscribe_int)(uint8_t *bit_no);

int (verify_stat_error)();

/*
*  KBC functions to issue commands, read data/command byte, write arguments
*/

int (kbc_issue_cmd)(uint8_t cmd);//Issues a given command to the kbc

int (kbc_write_argument)(uint32_t arg); //Writes arguments to be used by the write command

int (kbc_read_return) (uint32_t *arg); //Reads a kbc command's return data

uint32_t (kbc_read_command)(); //Returns the kbc's command byte

void (default_kbc)(); //Resets the kbc's command byte to minix defaults
