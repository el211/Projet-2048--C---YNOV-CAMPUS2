#include "render.h"

#include <SDL.h>

#include <stdint.h>

//ici je defini les dimension du plateau et des tuiles avec des #define pour pas mettre des nombre en dur
#define BOARD_X 30      //position x du plateau
#define BOARD_Y 150     //position y du plateau
#define BOARD_SIZE 500  //taille du plateau (carré)
#define GAP 12          //l'espace entre les tuiles
#define TILE_SIZE ((BOARD_SIZE - GAP * (GRID_SIZE + 1)) / GRID_SIZE) //taille d'une tuile calculer automatiquement

//petite structure pour une couleur (rouge vert bleu)
typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

//comme jutilise pas de police d'ecriture, jai dessiner mes chiffre moi meme
//chaque chiffre fait 3 colone de large et 5 ligne de haut, chaque ligne est coder sur 3 bits
//un bit a 1 = un carré allumer, un bit a 0 = rien
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

//petite fonction pour choisir la couleur du crayon avant de dessiner
static void set_color(struct SDL_Renderer *renderer, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
}

//cette fonction dessine un rectangle plein d'une couleur (je m'en sert tout le temps)
static void fill_rect(struct SDL_Renderer *renderer, int x, int y, int w, int h, Color color) {
    SDL_Rect rect = {x, y, w, h};
    set_color(renderer, color);
    SDL_RenderFillRect(renderer, &rect);
}

//cette fonction dessine UN chiffre en lisant les bits du tableau DIGITS
static void draw_digit(struct SDL_Renderer *renderer, int digit, int x, int y, int scale, Color color) {
    set_color(renderer, color);
    for (int row = 0; row < 5; ++row) {     //5 ligne
        for (int column = 0; column < 3; ++column) { //3 colone
            //la je test si le bit est a 1, si oui je dessine un petit carré (scale = la taille)
            if ((DIGITS[digit][row] >> (2 - column)) & 1U) {
                SDL_Rect pixel = {x + column * scale, y + row * scale, scale, scale};
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

//cette fonction compte combien y'a de chiffre dans un nombre (ex: 128 -> 3)
static int digit_count(int value) {
    int count = 1;
    while (value >= 10) {
        value /= 10; //je divise par 10 a chaque fois jusqua qu'il reste plus rien
        ++count;
    }
    return count;
}

//cette fonction affiche un nombre bien centrer dans un rectangle donner
static void draw_number_centered(struct SDL_Renderer *renderer, int value, SDL_Rect bounds,
                                 int max_scale, Color color) {
    int digits = digit_count(value);
    int scale = max_scale;
    int width;

    //je reduit la taille (scale) tant que le nombre rentre pas dans le cadre
    do {
        width = digits * 3 * scale + (digits - 1) * scale;
        if (width <= bounds.w - 12 && 5 * scale <= bounds.h - 12) {
            break; //la ca rentre donc je m'arrete
        }
        --scale;
    } while (scale > 1);

    //je calcule la position de depart pour que le nombre soit pile au milieu
    int x = bounds.x + (bounds.w - width) / 2;
    int y = bounds.y + (bounds.h - 5 * scale) / 2;
    int divisor = 1;
    for (int i = 1; i < digits; ++i) {
        divisor *= 10; //ca me sert a recuperer les chiffre un par un (centaine, dizaine, unité...)
    }

    //je dessine chaque chiffre de gauche a droite
    while (divisor > 0) {
        draw_digit(renderer, (value / divisor) % 10, x, y, scale, color);
        x += 4 * scale;  //je decale a droite pour le prochain chiffre
        divisor /= 10;
    }
}

//cette fonction renvoi la couleur d'une tuile selon sa valeur (comme dans le vrai 2048)
static Color tile_color(int value) {
    switch (value) {
        case 0: return (Color){205, 193, 180};    //case vide
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
        default: return (Color){237, 194, 46};    //2048 et plus -> jaune doré
    }
}

//cette fonction dessine TOUT l'ecran (c'est elle qu'on appelle depuis le main)
void draw_game(struct SDL_Renderer *renderer, const Game *game) {
    fill_rect(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){250, 248, 239}); //le fond

    //je dessine le titre "2048" en haut a gauche
    SDL_Rect title_bounds = {28, 32, 230, 76};
    draw_number_centered(renderer, 2048, title_bounds, 12, (Color){119, 110, 101});

    //la case du score en haut a droite avec le score dedans
    fill_rect(renderer, 340, 35, 190, 74, (Color){187, 173, 160});
    SDL_Rect score_bounds = {348, 42, 174, 60};
    draw_number_centered(renderer, game->score, score_bounds, 7, (Color){255, 255, 255});

    //je dessine le grand plateau gris
    fill_rect(renderer, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE, (Color){187, 173, 160});
    //puis je parcour chaque case pour dessiner les tuiles
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            //je calcule la position de la tuile en fonction de la ligne et la colone
            int x = BOARD_X + GAP + column * (TILE_SIZE + GAP);
            int y = BOARD_Y + GAP + row * (TILE_SIZE + GAP);
            int value = game->cells[row][column];
            fill_rect(renderer, x, y, TILE_SIZE, TILE_SIZE, tile_color(value)); //le fond de la tuile

            if (value != 0) { //si la case est pas vide jecrit le nombre dessus
                //texte foncé pour 2 et 4 (couleur claire) sinon texte clair
                Color text = value <= 4 ? (Color){119, 110, 101} : (Color){249, 246, 242};
                SDL_Rect tile_bounds = {x, y, TILE_SIZE, TILE_SIZE};
                draw_number_centered(renderer, value, tile_bounds, 10, text);
            }
        }
    }

    //si on a gagner ou perdu je met un voile transparent par dessus le plateau
    if (game->won || game->game_over) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); //le mode blend = transparence
        fill_rect(renderer, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE,
                  game->won ? (Color){237, 194, 46} : (Color){238, 228, 218});
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        //jaffiche 2048 si gagner, sinon le score final si perdu
        SDL_Rect message = {BOARD_X + 80, BOARD_Y + 180, BOARD_SIZE - 160, 140};
        draw_number_centered(renderer, game->won ? 2048 : game->score, message, 18,
                             (Color){119, 110, 101});
    }

    SDL_RenderPresent(renderer); //a la fin j'affiche tout a l'ecran d'un coup
}
