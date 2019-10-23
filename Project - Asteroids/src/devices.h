#pragma once
#include <lcom/lcf.h>

/** @defgroup devices devices
* @{
*	All code relating to low level keyboard, mouse, timer 0 and RTC devices
*/

 /* Data structures */

/** @brief Device interrupt type */
typedef enum {KEYBOARD, MOUSE, TIMER, SERIALPORT, RTC} event_type;
/** @brief Keyboard events */
typedef enum {MAIN_THRUSTER , PORT_THRUSTER, STARBOARD_THRUSTER, REVERSE_THRUSTER, K_ESC, QUIT, IDLING} kb_evt_tp;

/** @brief RTC time struct */
typedef struct {
	uint8_t seconds;						/**< @brief RTC seconds in binary format */
	uint8_t minutes;						/**< @brief RTC minutes in binary format */	
	uint8_t hours;							/**< @brief RTC hours in binary format */
} rtc_time_t;

/** @brief Various game related timers */
typedef struct {
	unsigned int framecounter;				/**< @brief Counts every frame displayed on screen */
	unsigned int frames_per_second;			/**< @brief Saves the amount of frames displayed every second */
	unsigned int cyclecounter;				/**< @brief Counts the number of cycles done while playing */
	unsigned int timerTick;					/**< @brief Counts the number of timer 0 interrupts */
	unsigned int player1_weapon_timer;		/**< @brief Controls player1 fire rate */
	unsigned int player2_weapon_timer;		/**< @brief Controls player2 fire rate */
	unsigned int alien_weapon_timer;		/**< @brief Controls alien fire rate */
	unsigned int alien_death_timer;			/**< @brief Used for alien death animation */
	unsigned int teleport_timer;			/**< @brief Controls player1 teleport availability rate */		
	unsigned int round_timer;				/**< @brief Counts time passed in round */
	int start_seq;							/**< @brief Single player start sequence countdown */
}game_timers;

 /* Functions */

/**
 * @brief Enables the mouse's data reporting
 * @return Return 0 upon success and non-zero otherwise
 */
int (enable_data_reporting) ();
/**
 * @brief Subscribes to mouse interrupts and calls enable_data_reporting()
 */
void mouse_init ();
/**
 * @brief Disables the mouse's data reporting
 * @return Return 0 upon success and non-zero otherwise
 */
int (disable_data_reporting) ();
/**
 * @brief Unsubscribes to mouse interrupts and calls disable_data_reporting()
 */
void mouse_stop ();
/**
 * @brief Subscribes to keyboard interrupts
 */
void keyboard_subscribe ();
/**
 * @brief Unsubscribes to keyboard interrupts
 */
void keyboard_unsubscribe ();
/**
 * @brief Subscribes to timer 0 interrupts
 */
void timer_subscribe ();
/**
 * @brief Unsubscribes to timer 0 interrupts
 */
void timer_unsubscribe ();
/**
 * @brief Subscribes to RTC interrupts
 */
void rtc_subscribe();
/**
 * @brief Unsubscribes to RTC interrupts
 */
void rtc_unsubscribe();
/**
 * @brief Enables Updated-ended RTC interrupts
 */
void rtc_enable_int();
/**
 * @brief Disables Updated-ended RTC interrupts
 */
void rtc_disable_int();
/**
 * @brief Subscribes and enables mouse, keyboard, timer 0, rtc, serial port interrupts
 */
void init_devices();
/**
 * @brief Unsubscribes and disables mouse, keyboard, timer 0, rtc, serial port interrupts
 */
void reset_devices();

/* Interrupts Handlers */

/**
 * @brief Timer 0 Interrupt handler, updates the game's various timers aswell as each asteroid's death timer
 *
 * @param timers Game timers
 */
void timer_interrupt_handler (game_timers *timers);
/**
 * @brief Parses mouse packets into a struct
 *
 * @param byte1 First byte from packet
 * @param byte2 Second byte from packet
 * @param byte3 Third byte from packet
 * @returns Returns parsed mouse packet struct
 */
struct packet (parse_mpacket) (uint8_t byte1, uint8_t byte2, uint8_t byte3); //parses mouse packets
/**
 * @brief Reads mouse input data, then calls parse_mpacket() to parse the packet
 *
 * Uses inline assembly
 *
 * @param ms_ev Pointer to mouse packet struct
 * @returns Returns 1 on the third byte (packet complete), 0 otherwise
 */
int mouse_interrupt_handler(struct packet *ms_ev);

/**
 * @brief Reads the keyboard's scan code, and updates the keyboard game event enum
 *
 * Uses inline assembly
 *
 * @param evt Pointer to keyboard enum
 */
void keyboard_interrupt_handler (kb_evt_tp *evt);

/* KBC Auxiliary functions */

/**
 * @brief Issues a command to the KBC
 *
 * Uses inline assembly
 *
 * @param cmd Command to be issued
 * @returns Return 0 upon success and non-zero otherwise
 */
int (kbc_issue_cmd)(uint8_t cmd);
/**
 * @brief Writes an argument to the output buffer
 *
 * Uses inline assembly
 *
 * @param arg Argument to be written
 * @returns Return 0 upon success and non-zero otherwise
 */
int (kbc_write_argument)(uint32_t arg);
/**
 * @brief Reads the return of a KBC command
 *
 * Uses inline assembly
 *
 * @param arg Return of KBC command is written here
 * @returns Returns 0 upon success and non-zero otherwise
 */
int (kbc_read_return) (uint32_t *arg); 
/**
 * @brief Reads the return of the KBC command byte
 *
 * Uses inline assembly
 *
 * @returns Returns the KBC's command byte
 */
uint32_t (kbc_read_command)();

/**
 * @brief Checks for erros in the KBC's status register
 *
 * Uses inline assembly
 *
 * @returns Returns 0 on success, 1 if it took too many tries
 */
int (verify_stat_error)();

/* RTC auxiliary functions */

/**
 * @brief Converts number from BCD to binary
 *
 * @param date BCD number to be converted
 * @returns Returns binary format
 */
uint8_t readBCD(uint8_t date);
/**
 * @brief Reads from a RTC register
 *
 * @param ADDR Address of register to be read
 * @returns Returns data from register
 */
uint8_t readRTC(uint32_t ADDR);
/**
 * @brief Writes to a RTC register
 *
 * @param ADDR Address of register to be written to
 * @param data Data to write on register
 */
void writeRTC(uint32_t ADDR, uint32_t data);
/**
 * @brief Reads RTC's hours, minutes and seconds and saves it on the RTC time struct
 *
 * @param timer RTC time struct
 */
void readTime(rtc_time_t *time);



