#include "SDL.h"
#include "chip8.c"
#include "main.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

u8_t init_sdl(sdl_t *sdl, int scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf("Unable to init SDL: %s", SDL_GetError());
        return -1;
    }

    sdl->window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 64 * scale, 32 * scale, 0);
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

void update_display(sdl_t *sdl) {
    SDL_RenderClear(sdl->renderer);
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
        printf("Specify rom name when running.\n\tEx.: main <path/to/rom_name>\n");
        return -1;
    }

    char *rom_name = argv[1];
    int scale = 20;

    sdl_t sdl;
    if (init_sdl(&sdl, scale) != 0) {
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

        u8_t r, g, b, a;
        SDL_GetRenderDrawColor(sdl.renderer, &r, &g, &b, &a);
        if (SDL_SetRenderDrawColor(sdl.renderer, r + 3, g + 8, b + 14, a) != 0) {
        printf("SDL could not set render color: %s\n", SDL_GetError());
        return -1;
    }

        update_display(&sdl);
    }

    clean_up_sdl(&sdl);
    return 0;
}