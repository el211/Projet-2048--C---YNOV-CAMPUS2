#include <SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cmake-build-debug/_deps/sdl2-src/include/SDL_rect.h"

#define GRID_SIZE 4
#define WINDOW_WIDTH 560
#define WINDOW_HEIGHT 700
#define BOARD_X 30
#define BOARD_Y 150
#define BOARD_SIZE 500
#define GAP 12
#define TILE_SIZE ((BOARD_SIZE - GAP * (GRID_SIZE + 1)) / GRID_SIZE)

typedef enum {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN
} MoveDirection;

typedef struct {
    int cells[GRID_SIZE][GRID_SIZE];
    int score;
    bool won;
    bool game_over;
} Game;

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

static const uint8_t DIGITS[10][5] = {
    {0x7, 0x5, 0x5, 0x5, 0x7},
    {0x2, 0x6, 0x2, 0x2, 0x7},
    {0x7, 0x1, 0x7, 0x4, 0x7},
    {0x7, 0x1, 0x7, 0x1, 0x7},
    {0x5, 0x5, 0x7, 0x1, 0x1},
    {0x7, 0x4, 0x7, 0x1, 0x7},
    {0x7, 0x4, 0x7, 0x5, 0x7},
    {0x7, 0x1, 0x1, 0x1, 0x1},
    {0x7, 0x5, 0x7, 0x5, 0x7},
    {0x7, 0x5, 0x7, 0x1, 0x7}
};

static void set_color(struct SDL_Renderer *renderer, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
}

static void fill_rect(struct SDL_Renderer *renderer, int x, int y, int w, int h, Color color) {
    SDL_Rect rect = {x, y, w, h};
    set_color(renderer, color);
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_digit(struct SDL_Renderer *renderer, int digit, int x, int y, int scale, Color color) {
    set_color(renderer, color);
    for (int row = 0; row < 5; ++row) {
        for (int column = 0; column < 3; ++column) {
            if ((DIGITS[digit][row] >> (2 - column)) & 1U) {
                SDL_Rect pixel = {x + column * scale, y + row * scale, scale, scale};
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

static int digit_count(int value) {
    int count = 1;
    while (value >= 10) {
        value /= 10;
        ++count;
    }
    return count;
}

static void draw_number_centered(struct SDL_Renderer *renderer, int value, SDL_Rect bounds,
                                 int max_scale, Color color) {
    int digits = digit_count(value);
    int scale = max_scale;
    int width;

    do {
        width = digits * 3 * scale + (digits - 1) * scale;
        if (width <= bounds.w - 12 && 5 * scale <= bounds.h - 12) {
            break;
        }
        --scale;
    } while (scale > 1);

    int x = bounds.x + (bounds.w - width) / 2;
    int y = bounds.y + (bounds.h - 5 * scale) / 2;
    int divisor = 1;
    for (int i = 1; i < digits; ++i) {
        divisor *= 10;
    }

    while (divisor > 0) {
        draw_digit(renderer, (value / divisor) % 10, x, y, scale, color);
        x += 4 * scale;
        divisor /= 10;
    }
}

static Color tile_color(int value) {
    switch (value) {
        case 0: return (Color){205, 193, 180};
        case 2: return (Color){238, 228, 218};
        case 4: return (Color){237, 224, 200};
        case 8: return (Color){242, 177, 121};
        case 16: return (Color){245, 149, 99};
        case 32: return (Color){246, 124, 95};
        case 64: return (Color){246, 94, 59};
        case 128: return (Color){237, 207, 114};
        case 256: return (Color){237, 204, 97};
        case 512: return (Color){237, 200, 80};
        case 1024: return (Color){237, 197, 63};
        default: return (Color){237, 194, 46};
    }
}

static void add_random_tile(Game *game) {
    int empty[GRID_SIZE * GRID_SIZE][2];
    int empty_count = 0;

    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            if (game->cells[row][column] == 0) {
                empty[empty_count][0] = row;
                empty[empty_count][1] = column;
                ++empty_count;
            }
        }
    }

    if (empty_count == 0) {
        return;
    }

    int selected = rand() % empty_count;
    game->cells[empty[selected][0]][empty[selected][1]] = (rand() % 10 == 0) ? 4 : 2;
}

static bool can_move(const Game *game) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            if (game->cells[row][column] == 0) {
                return true;
            }
            if (column + 1 < GRID_SIZE &&
                game->cells[row][column] == game->cells[row][column + 1]) {
                return true;
            }
            if (row + 1 < GRID_SIZE &&
                game->cells[row][column] == game->cells[row + 1][column]) {
                return true;
            }
        }
    }
    return false;
}

static bool process_line(int line[GRID_SIZE], int *score) {
    int original[GRID_SIZE];
    int compacted[GRID_SIZE] = {0};
    int output[GRID_SIZE] = {0};
    int compacted_count = 0;
    int output_count = 0;

    for (int i = 0; i < GRID_SIZE; ++i) {
        original[i] = line[i];
        if (line[i] != 0) {
            compacted[compacted_count++] = line[i];
        }
    }

    for (int i = 0; i < compacted_count; ++i) {
        if (i + 1 < compacted_count && compacted[i] == compacted[i + 1]) {
            output[output_count] = compacted[i] * 2;
            *score += output[output_count];
            ++output_count;
            ++i;
        } else {
            output[output_count++] = compacted[i];
        }
    }

    bool changed = false;
    for (int i = 0; i < GRID_SIZE; ++i) {
        line[i] = output[i];
        if (line[i] != original[i]) {
            changed = true;
        }
    }
    return changed;
}

static bool move_tiles(Game *game, MoveDirection direction) {
    bool changed = false;

    for (int index = 0; index < GRID_SIZE; ++index) {
        int line[GRID_SIZE];
        for (int position = 0; position < GRID_SIZE; ++position) {
            int row;
            int column;
            if (direction == MOVE_LEFT || direction == MOVE_RIGHT) {
                row = index;
                column = direction == MOVE_LEFT ? position : GRID_SIZE - 1 - position;
            } else {
                row = direction == MOVE_UP ? position : GRID_SIZE - 1 - position;
                column = index;
            }
            line[position] = game->cells[row][column];
        }

        if (process_line(line, &game->score)) {
            changed = true;
        }

        for (int position = 0; position < GRID_SIZE; ++position) {
            int row;
            int column;
            if (direction == MOVE_LEFT || direction == MOVE_RIGHT) {
                row = index;
                column = direction == MOVE_LEFT ? position : GRID_SIZE - 1 - position;
            } else {
                row = direction == MOVE_UP ? position : GRID_SIZE - 1 - position;
                column = index;
            }
            game->cells[row][column] = line[position];
        }
    }

    if (changed) {
        add_random_tile(game);
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int column = 0; column < GRID_SIZE; ++column) {
                if (game->cells[row][column] >= 2048) {
                    game->won = true;
                }
            }
        }
        game->game_over = !can_move(game);
    }
    return changed;
}

static void reset_game(Game *game) {
    *game = (Game){0};
    add_random_tile(game);
    add_random_tile(game);
}

static void draw_game(struct SDL_Renderer *renderer, const Game *game) {
    fill_rect(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){250, 248, 239});

    SDL_Rect title_bounds = {28, 32, 230, 76};
    draw_number_centered(renderer, 2048, title_bounds, 12, (Color){119, 110, 101});

    fill_rect(renderer, 340, 35, 190, 74, (Color){187, 173, 160});
    SDL_Rect score_bounds = {348, 42, 174, 60};
    draw_number_centered(renderer, game->score, score_bounds, 7, (Color){255, 255, 255});

    fill_rect(renderer, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE, (Color){187, 173, 160});
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            int x = BOARD_X + GAP + column * (TILE_SIZE + GAP);
            int y = BOARD_Y + GAP + row * (TILE_SIZE + GAP);
            int value = game->cells[row][column];
            fill_rect(renderer, x, y, TILE_SIZE, TILE_SIZE, tile_color(value));

            if (value != 0) {
                Color text = value <= 4 ? (Color){119, 110, 101} : (Color){249, 246, 242};
                SDL_Rect tile_bounds = {x, y, TILE_SIZE, TILE_SIZE};
                draw_number_centered(renderer, value, tile_bounds, 10, text);
            }
        }
    }

    if (game->won || game->game_over) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        fill_rect(renderer, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE,
                  game->won ? (Color){237, 194, 46} : (Color){238, 228, 218});
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Rect message = {BOARD_X + 80, BOARD_Y + 180, BOARD_SIZE - 160, 140};
        draw_number_centered(renderer, game->won ? 2048 : game->score, message, 18,
                             (Color){119, 110, 101});
    }

    SDL_RenderPresent(renderer);
}

static bool direction_from_key(SDL_Keycode key, MoveDirection *direction) {
    switch (key) {
        case SDLK_LEFT:
        case SDLK_a:
        case SDLK_q:
            *direction = MOVE_LEFT;
            return true;
        case SDLK_RIGHT:
        case SDLK_d:
            *direction = MOVE_RIGHT;
            return true;
        case SDLK_UP:
        case SDLK_w:
        case SDLK_z:
            *direction = MOVE_UP;
            return true;
        case SDLK_DOWN:
        case SDLK_s:
            *direction = MOVE_DOWN;
            return true;
        default:
            return false;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Impossible d'initialiser SDL2 : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow(
        "2048 - SDL2 | Fleches/ZQSD/WASD, R: recommencer, Echap: quitter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Impossible de creer la fenetre : %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) {
        fprintf(stderr, "Impossible de creer le renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    Game game;
    reset_game(&game);
    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    running = false;
                } else if (key == SDLK_r || key == SDLK_n) {
                    reset_game(&game);
                } else if (!game.game_over && !game.won) {
                    MoveDirection direction;
                    if (direction_from_key(key, &direction)) {
                        move_tiles(&game, direction);
                    }
                }
            }
        }

        draw_game(renderer, &game);
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
