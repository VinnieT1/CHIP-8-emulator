#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * @brief Memory size in bytes
 */
#define MEMORY_SIZE 4096

/**
 * @brief Total number of registers
 */
#define NUMBER_OF_REGISTERS 16

/**
 * @brief Numer of pixels in vertical direction
 */
#define PIXEL_ROWS 32

/**
 * @brief Number of pixels in horizontal direction
 */
#define PIXEL_COLUMNS 64

/**
 * @brief Number of keys in keypad
 */
#define NUMBER_OF_KEYS 16

/**
 * @brief Size of stack (16 16-bits)
 */
#define STACK_SIZE 16

/**
 * @brief Unsigned byte (8 bits)
 */
typedef unsigned char u8_t;

/**
 * @brief Two unsigned bytes (16 bits)
 */
typedef unsigned short u16_t;

/**
 * @brief starting index in memory where roms are loaded
 */
#define ROM_INDEX_START 0x200

/**
 * @brief CHIP8 state: if game is running, paused or quitting.
 */
typedef enum {
    QUIT,
    RUNNING,
    PAUSED,
} chip8_state_t;

/**
 * @brief CHIP8 type: emulates CHIP8 components
 */
typedef struct chip8 {
	char *rom_name;

	chip8_state_t state;

    u16_t opcode;
    u16_t I;
    u16_t pc;
    u8_t sp;

    u16_t stack[STACK_SIZE];
    u8_t memory[MEMORY_SIZE];
    u8_t V[NUMBER_OF_REGISTERS];
    u8_t pixels[PIXEL_COLUMNS * PIXEL_ROWS];
    u8_t keypad[NUMBER_OF_KEYS];

    u8_t delay_timer;
    u8_t sound_timer;
} chip8_t;

u8_t chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

/**
 * @brief Initializes chip8 components.
 *
 * @param[in] chip8 Chip8 being emulated.
 * @return int 0 in case of success and and a negative integer in case of failure.
 */
int chip8_init(chip8_t *chip8, char *rom_name);

/**
 * @brief Emulates a chip8 cycle.
 *
 * @param[in] chip8 Chip8 being emulated.
 * @return int 0 in case of success and and a negative integer in case of failure.
 */
int chip8_cycle(chip8_t *chip8);

/**
 * @brief Fetches opcode from memory in the indexes pc and pc + 1 and stores
 * it into chip8 opcode.
 *
 * @param[in] chip8 Chip8 being emulated.
 * @return int 0 in case of success and and a negative integer in case of failure.
 */
int chip8_fetch_opcode(chip8_t *chip8);

/**
 * @brief Execute instruction based on opcode previously fetched by fetch_opcode.
 *
 * @param[in] chip8 Chip8 being emulated.
 * @return int 0 in case of success and and a negative integer in case of failure.
 */
int chip8_execute_instruction(chip8_t *chip8);

/**
 * @brief Updates chip8 timers.
 *
 * @param[in] chip8 Chip8 being emulated.
 * @return int 0 in case of success and and a negative integer in case of failure.
 */
int chip8_update_timers(chip8_t *chip8);

/**
 * @brief Increments PC to point to next instruction.
 *
 * @param[in] chip8 Chip8 being emulated.
 * @return int 0 in case of success and and a negative integer in case of failure.
 */
int chip8_increment_pc(chip8_t *chip8);

int chip8_load_rom(chip8_t *chip8);

#endif