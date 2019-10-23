#pragma once

/*
* Serial port hook id -> bit 0 = 0x01
* Timer hook id -> bit 1 = 0x02
* Mouse hook id -> bit 2 = 0x04
* Keyboard hook id -> bit 3 = 0x08
* RTC hook id -> bit 4 = 0x10
*/

/* GAME MACROS */

//Technical
#define PHYSICS_TICKS 1 //1 - 60 ticks/s(recommended), 2 - 30 ticks/s, 3 - 20 tick/s, 4- 15 ticks/s, 5 12 ticks/s
#define MOUSE_SENSITIVITY 1 //Crosshair speed

//Ship
#define FIRE_RATE 3 //Lasers per second
#define JUMP_RATE 3 //Seconds before another random jump is available
#define MAIN_THRUSTER_ACCELARATION 2
#define MAIN_REVERSE_ACCELARATION 0.5
#define PORT_THRUSTER_ACCELARATION 0.5
#define STARBOARD_THRUSTER_ACCELARATION 0.5
#define THRUSTERS_MAXIMUM_VELOCITY 4 //Overall ship's maximum velocity
#define SHIP_HITRADIUS 17
#define AMMO 10 //Maximum bullets on screen at any moment
#define LASER_VELOCITY 8 //Projectile travel speed
#define PLAYER_MAX_HEALTH 100 //Maximum player health points
#define PLAYER_HEALTH_REGENERATION 10 //Player health points gain per round
#define PLAYER_LASER_DAMAGE 30

//Alien ship
#define ALIEN_MAX_HEALTH 150
#define ALIEN_FIRE_RATE 1.5
#define ALIEN_HIT_RADIUS 18
#define ALIEN_MAX_ACCELARATION 2
#define ALIEN_MIN_ACCELARATION 1
#define ALIEN_MAX_VELOCITY 3
#define ALIEN_DEATH_DURATION 0.5
#define ALIEN_SPAWN_CHANCE_INCREASE 20
#define ALIEN_ROTATION_SPEED 2

#define DELAY_BETWEEN_ROUNDS 60

//Asteroids
#define MAX_ASTEROIDS 20 //Maximum number of asteroids on screen at any time
#define STARTING_ASTEROIDS 3 //Number of asteroids on the first round
#define ASTEROID_INCREASE_RATE 1 //Increase in asteroid count per round
#define SMALL_ASTEROID_CHANGE_INCREASE_RATE 3 //Increase in chance of small asteroid every round
#define ASTEROID_DEATH_DURATION 0.5 //Durantion of asteroid destruction animation in seconds
#define MEDIUM_ASTEROID_HITRADIUS 20
#define MEDIUM_ASTEROID_MAX_VELOCITY 4
#define MEDIUM_ASTEROID_MIN_VELOCITY 3
#define LARGE_ASTEROID_HITRADIUS 28
#define LARGE_ASTEROID_MAX_VELOCITY 2
#define LARGE_ASTEROID_MIN_VELOCITY 1

/* DEVICE MACROS */

//General
#define negative_16 0xFF00
#define abs_16 0x00FF
#define BIT(n) (0x01<<(n))

//Tick delay
#define DELAY_US 20000

//Video card functions
#define set_vbe_mode 0x4F02
#define get_vbe_info 0x4F01
#define video_card_bios 0x10
#define Direct24_1024 0x118
#define vbe_success_return 0x4F
#define minix_text_mode 0x0003
#define COLOR_IGNORED 0xFFAEC9

//1024x768 bounds
#define hres 1024
#define vres 768
#define math_h_negative_bound -512
#define math_h_positive_bound 511
#define math_v_negative_bound -383
#define math_v_positive_bound 384

//Mouse packet
#define y_ovf BIT(7)
#define x_ovf BIT(6)
#define y_sign BIT(5)
#define x_sign BIT(4)
#define byte1_fixed BIT(3)
#define mouse_mb BIT(2)
#define mouse_rb BIT(1)
#define mouse_lb BIT(0)
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

//KBC 
#define OUT_BUF 0x60
#define STAT_REG 0x64
#define IBF BIT(1)
#define OBF BIT(0)
#define PAR_ERR BIT(7)
#define TO_ERR BIT(6)
#define kbc_write_cmd 0x60

//RTC 
#define RTC_ADDR_REG	0x70
#define RTC_DATA_REG	0x71
#define REGISTER_A		0x0A
#define REGISTER_B		0x0B
#define REGISTER_C		0x0C
#define REGISTER_D		0x0D
#define RTC_HRS			0x04
#define RTC_MIN			0x02
#define RTC_SEC			0x00

//Serial port
#define COM1_ADDR 0x3F8
#define COM2_ADDR 0x2F8
#define SP_IH_ENABLE 1
#define SP_INT_IDENTIFICATION 2
#define SP_FIFO_CONTROL 2
#define SP_LINE_CTRL 3
#define SP_LINE_STATUS 5
#define SP_INT_ORIGIN 0x0E
#define DLAB 0x80
#define DL_LSB 0
#define DL_MSB 1
#define STOP_BITS_2 0x04
#define PARITY_EVEN 0x18
#define WORD_LENGTH_8 0x03
#define SP_RATE 115200
#define SP_FIFO_INIT 0x07
#define SP_FIFO_READY 0x01
#define SP_TX_READY 0x20
#define SP_RX_READY 0x01


//IRQ Sets
#define irq_com1_serial BIT(0)
#define irq_timer0 BIT(1)
#define irq_mouse BIT(2)
#define irq_keyboard BIT(3)
#define irq_rtc BIT(4)
#define irq_com2_serial BIT(5)

//IRQ Lines
#define kb_irq_line 1
#define timer_irq_line 0
#define mouse_irq_line 12
#define rtc_irq_line 8
#define serial_com1_irq_line 4
#define serial_com2_irq_line 3

