#include "chip8.h"

int chip8_init(chip8_t *chip8, char *rom_name) {
    chip8->opcode = 0;
    chip8->I = 0;
    chip8->pc = 0x200;
    chip8->sp = 0;
    chip8->rom_name = rom_name;
    chip8->state = RUNNING;

    memset(chip8->stack, 0, STACK_SIZE * sizeof(u16_t));
    memset(chip8->memory, 0, MEMORY_SIZE * sizeof(u8_t));
    memset(chip8->V, 0, NUMBER_OF_REGISTERS * sizeof(u8_t));
    memset(chip8->pixels, 0, PIXEL_COLUMNS * PIXEL_ROWS * sizeof(u8_t));
    memset(chip8->keypad, 0, NUMBER_OF_KEYS * sizeof(u8_t));

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    for (u8_t i = 0; i < 80; i++) {
        chip8->memory[i] = chip8_fontset[i];
    }

    if (chip8_load_rom(chip8) != 0) {
        printf("Could not load ROM\n");
        return -1;
    }

    return 0;
}

int chip8_fetch_opcode(chip8_t *chip8) {
    u8_t *memory = chip8->memory;
    u16_t pc = chip8->pc;

    chip8->opcode = ((u16_t)memory[pc] << 8) | (memory[pc + 1]);

    return 0;
}

int chip8_increment_pc(chip8_t *chip8) {
    chip8->pc += 2;

    return 0;
}

int chip8_cycle(chip8_t *chip8) {
    return (
        chip8_fetch_opcode(chip8) || 
        chip8_execute_instruction(chip8) ||
        chip8_increment_pc(chip8) ||
        chip8_update_timers(chip8)
    );
}

int chip8_execute_instruction(chip8_t *chip8) {
    // opcode can be read as 0x zxyn, with z bein the first nibble;
    // or even 0x zxkk
    u16_t opcode = chip8->opcode;

    // gets K from a 16bit value like 0xKXXX and transforms into 0x000K or just 0xK
    u8_t first_opcode_nibble = opcode >> 12;

    // useful value
    u8_t code = opcode & 0x000F;

    // choose registers
    u8_t x = (opcode & 0x0F00) >> 8;
    u8_t y = (opcode & 0x00F0) >> 4;

    // can be immediate value and can be used to select instruction
    u8_t kk = (opcode & 0x00FF);

    // useful reference of registers
    u8_t *V = chip8->V;
    printf("Address: 0x%04X, Opcode: 0x%04X, ", chip8->pc, opcode);
    switch (first_opcode_nibble) {
        case 0x0:
            // Clear the display.
            if (opcode == 0x00E0) {
                printf("Clear the display\n");
                memset(chip8->pixels, 0, PIXEL_COLUMNS * PIXEL_ROWS * sizeof(u8_t));
            }
            // Return from a subroutine.
            else if (opcode == 0x00EE) {
                printf("Return from a subroutine\n");
                chip8->sp--;
                chip8->pc = chip8->stack[chip8->sp] - 2;
            }
            // Jump to a machine code routine at nnn. 0x0nnn
            else {
                // printf("Not implemented\n");
                chip8->pc = (opcode & 0x0FFF) - 2;
            }
            break;
        case 0x1:
            // Jump to location nnn.
            printf("Jump to location 0x%04X\n", opcode & 0x0FFF);
            chip8->pc = (opcode & 0x0FFF) - 2;
            break;
        case 0x2:
            // Call subroutine at nnn.
            printf("Call subroutine at 0x%04X\n", opcode & 0x0FFF);
            chip8->memory[chip8->sp] = chip8->pc;
            chip8->sp += 1;
            chip8->pc = opcode & 0x0FFF;
            break;
        case 0x3:
            // Skip next instruction if Vx = kk.
            if (chip8->V[x] == kk) {
                chip8->pc += 2;
            }
            break;
        case 0x4:
            // Skip next instruction if Vx != kk.
            if (chip8->V[x] != kk) {
                chip8->pc += 2;
            }
            break;
        case 0x5:
            // Skip next instruction if Vx = Vy.
            if (chip8->V[x] == chip8->V[y]) {
                chip8->pc += 2;
            }
            break;
        case 0x6:
            // Set Vx = kk.
            printf("Set V[%01X] = %d\n", x, (int)kk);
            chip8->V[x] = kk;
            break;
        case 0x7:
            // Set Vx = Vx + kk.
            printf("Set V[%X] = V[%X] (0x%X; %d) + 0x%X (%d) = %d or 0x%X\n", x, x, V[x], V[x], kk, kk, V[x] + kk, V[x] + kk);
            chip8->V[x] += kk;
            break;
        case 0x8:
            // Set Vx = Vy.
            if (code == 0x0000) {
                V[x] = V[y];
            }
            // Set Vx = Vx OR Vy.
            else if (code == 0x0001) {
                V[x] |= V[y];
            }
            // Set Vx = Vx AND Vy.
            else if (code == 0x0002) {
                V[x] &= V[y];
            }
            // Set Vx = Vx XOR Vy.
            else if (code == 0x0003) {
                V[x] ^= V[y];
            }
            // Set Vx = Vx + Vy, set VF = carry.
            else if (code == 0x0004) {
                V[x] += V[y];
                V[0xF] = (u16_t)V[x] + V[y] > 255;
            }
            // Set Vx = Vx - Vy, set VF = NOT borrow.
            else if (code == 0x0005) {
                V[0xF] = V[x] > V[y];
                V[x] -= V[y];
            }
            // Set Vx = Vx SHR 1.
            else if (code == 0x0006) {
                V[0xF] = V[x] & 0x01;
                V[x] >>= 1;
            }
            // Set Vx = Vy - Vx, set VF = NOT borrow.
            else if (code == 0x0007) {
                V[0xF] = V[y] > V[x];
                V[x] = V[y] - V[x];
            }
            // Set Vx = Vx SHL 1.
            else if (code == 0x0008) {
                V[0xF] = V[x] & 0x01;
                V[x] <<= 1;
            }
            break;
        case 0x9:
            // Skip next instruction if Vx != Vy.
            if (V[x] != V[y]) {
                chip8->pc += 2;
            }
            break;
        case 0xA:
            // Set I = nnn.
            printf("Set I = 0x%04X\n", opcode & 0x0FFF);
            chip8->I = opcode & 0x0FFF;
            break;
        case 0xB:
            // Jump to location nnn + V0.
            chip8->pc = (opcode & 0x0FFF) + V[0] - 2;
            break;
        case 0xC:
            // Set Vx = random byte AND kk.
            V[x] = (rand() % 256) & kk;
            break;
        case 0xD:
            // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            printf("Display %d-byte sprite starting at memory location I = 0x%04X at (V[%X] = %d, V[%X] = %d), set V[F] collision\n", code, chip8->I, x, V[x], y, V[y]);
            u16_t x_coord = V[x] % PIXEL_COLUMNS;
            u16_t y_coord = V[y] % PIXEL_ROWS;
            u16_t original_x = x_coord;
            V[0xF] = 0;
            
            for (u8_t i = 0; i < code; i++) {
                u8_t sprite_data = chip8->memory[chip8->I + i];
                x_coord = original_x;

                for (int8_t j = 7; j >= 0; j--) {
                    u8_t *pixel = &(chip8->pixels[y_coord * PIXEL_COLUMNS + x_coord]);
                    u8_t sprite_bit = (sprite_data & (1 << j));

                    if (sprite_bit && *pixel) {
                        chip8->V[0xF] = 1;
                    }
                    *pixel ^= sprite_bit;

                    if (++x_coord >= PIXEL_COLUMNS) {
                        break;
                    }
                }

                if (++y_coord >= PIXEL_ROWS) {
                    break;
                }
            }
            break;
        case 0xE:
            // Skip next instruction if key with the value of Vx is pressed.
            if (code == 0xE) {
                if (chip8->keypad[V[x]]) {
                    chip8->pc += 2;
                }
            }
            // Skip next instruction if key with the value of Vx is not pressed.
            else if (code == 0x1) {
                if (!chip8->keypad[V[x]]) {
                    chip8->pc += 2;
                }
            }
            break;
        case 0xF:
            // Set Vx = delay timer value.
            if (kk == 0x07) {
                V[x] = chip8->delay_timer;
            }
            else if (kk == 0x0A) {
                u8_t key = 0;

                while (1) {
                    if (chip8->keypad[key]) {
                        V[x] = key;
                        break;
                    }
                    key = (key + 1) % NUMBER_OF_KEYS;
                }
            }
            // Set delay timer = Vx.
            else if (kk == 0x15) {
                chip8->delay_timer = V[x];
            }
            // Set sound timer = Vx.
            else if (kk == 0x18) {
                chip8->sound_timer = V[x];
            }
            // Set I = I + Vx.
            else if (kk == 0x1E) {
                chip8->I += V[x];
            }
            // Set I = location of sprite for digit Vx.
            else if (kk == 0x29) {
                // TODO
            }
            // Store BCD representation of Vx in memory locations I, I+1, and I+2.
            else if (kk == 0x33) {
                chip8->memory[chip8->I] = V[x] / 100;
                chip8->memory[chip8->I + 1] = (V[x] / 10) % 10;
                chip8->memory[chip8->I + 2] = V[x] % 10;
            }
            else if (kk == 0x55) {
                // Store registers V0 through Vx in memory starting at location I.
                for (u8_t i = 0; i < x; i++) {
                    chip8->memory[chip8->I + i] = V[i];
                }
            }
            // Read registers V0 through Vx from memory starting at location I.
            else if (kk == 0x65) {
                for (u8_t i = 0; i <= x; i++) {
                    V[i] = chip8->memory[chip8->I + i];
                }
            }
            break;
        default:
            return -1;
    }

    return 0;
}

int chip8_update_timers(chip8_t *chip8) {
    if (chip8->delay_timer > 0) {
        chip8->delay_timer -= 1;
    }
    if (chip8->sound_timer > 0) {
        chip8->sound_timer -= 1;
    }
    return 0;
}

int chip8_load_rom(chip8_t *chip8) {
    FILE *rom = fopen(chip8->rom_name, "rb");
    if (rom == NULL) {
        return -1;
    }
    fseek(rom, 0, SEEK_END);

    long rom_size = ftell(rom);
    if (rom_size > MEMORY_SIZE - 0x200) {
        printf("ROM too big\n");
        return -1;
    }

    rewind(rom);
    
    fread(&chip8->memory[ROM_INDEX_START], rom_size, sizeof(u8_t), rom);
    fclose(rom);

    return 0;
}