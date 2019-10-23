#pragma once

//tick delay
#define DELAY_US 20000

//Mouse packet
#define y_ovf 0x80
#define x_ovf 0x40
#define y_sign 0x20
#define x_sign 0x10
#define mouse_mb 0x04
#define mouse_lb 0x01
#define mouse_rb 0x02
#define byte1_fixed 0x00000008
#define ACK 0xFA

//Mouse commands
#define writeToMouse 0xD4

#define enableDataReporting 0xF4
#define disableDataReporting 0xF5
#define mouseReset 0xFF
#define readData 0xEB
#define setStreamMode 0xEA
#define setDefaults 0xF6
#define requestStatus 0xE9

//Mouse Status
#define fxd0 0x80
#define fxd00 0x08
#define m_mode 0x40
#define dreport 0x20
#define sclng 0x10
#define left_b 0x04
#define middle_b 0x02
#define right_b 0x01


//General
#define negative_16 0xFF00
#define abs_16 0x00FF

//kbc 
#define OUT_BUF 0x60
#define IBF 0x02
#define OBF 0x01
#define STAT_REG 0x64
#define PAR_ERR 0x80
#define TO_ERR 0x40
#define kbc_write_command 0x60

//IRQs
#define irq_mouse 0x00000004
#define irq_timer0 0x00000002

//IRQ Lines
#define mouse_irq_line 12

