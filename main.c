#include "SDL.h"
#include "chip8.c"
#include "main.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int scale;
} sdl_t;

u8_t init_sdl(sdl_t *sdl) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf("Unable to init SDL: %s", SDL_GetError());
        return -1;
    }

    sdl->window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, PIXEL_COLUMNS * sdl->scale,  PIXEL_ROWS * sdl->scale, 0);
    if (sdl->window == NULL) {
        printf("SDL could not initialize window: %s\n", SDL_GetError());
        return -1;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl->renderer == NULL) {
        printf("SDL could not initialize renderer: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0) {
        printf("SDL could not set render color: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderClear(sdl->renderer) != 0) {
        printf("SDL could not clear renderer: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

void clean_up_sdl(sdl_t *sdl) {
    SDL_DestroyWindow(sdl->window);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_Quit();
}

void update_display(sdl_t *sdl, chip8_t *chip8) {
    SDL_Rect rect = {
        .h = sdl->scale,
        .w = sdl->scale,
        .x = 0,
        .y = 0
    };

    for (u16_t i = 0; i < PIXEL_COLUMNS * PIXEL_ROWS; i++) {
        rect.x = (i % PIXEL_COLUMNS) * sdl->scale;
        rect.y = (i / PIXEL_COLUMNS) * sdl->scale;

        if (chip8->pixels[i]) {
            SDL_SetRenderDrawColor(sdl->renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(sdl->renderer, &rect);
        }
        else {
            SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(sdl->renderer, &rect);
        }
    }

    SDL_RenderPresent(sdl->renderer);
}

int handle_input(sdl_t *sdl, chip8_t *chip8) {
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                chip8->state = QUIT;
                return 0;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        chip8->state = QUIT;
                        return 0;
                    case SDLK_SPACE:
                        if (chip8->state == RUNNING && event.key.state == SDL_PRESSED) {
                            chip8->state = PAUSED;
                            printf("EMULATION PAUSED\n");
                        }
                        else if (chip8->state == PAUSED && event.key.state == SDL_PRESSED) {
                            chip8->state = RUNNING;
                            printf("EMULATION RESUMED\n");
                        }
                        break;
                    case SDLK_LCTRL:
                        printf("State: %s\n", chip8->state == PAUSED ? "PAUSED" : "RUNNING");
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }

    return 0;
}

// main <rom_name>
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Specify rom name when running.\n\tEx.: main <path/to/rom_name>\nOr simply drag the ROM into the .exe");
        return -1;
    }

    char *rom_name = argv[1];
    int scale = 20;

    sdl_t sdl;
    sdl.scale = scale;
    if (init_sdl(&sdl) != 0) {
        printf("Init error: %s\n", SDL_GetError());
        return -1;
    }

    chip8_t chip8;
    if (chip8_init(&chip8, rom_name) != 0) {
        printf("Init error on chip8\n");
        return -1;
    }

    // Cycle
    while (1) {
        if (handle_input(&sdl, &chip8) != 0) {
            return -1;
        }

        if (chip8.state == QUIT) {
            printf("quitting!\n");
            break;
        }
        else if (chip8.state == PAUSED) {
            continue;
        }

        if (chip8_cycle(&chip8) != 0) {
            printf("Chip8 cycle error\n");
            return -1;
        }
        SDL_Delay(16);

        update_display(&sdl, &chip8);
    }

    clean_up_sdl(&sdl);
    return 0;
}