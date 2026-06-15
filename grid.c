#include "grid.h"

#include <stdlib.h>

//alloue la grille (tableau 2D de 4x4). renvoi false si une allocation rate
bool game_init(Game *game) {
    //j'alloue d'abord le tableau principal (les 4 lignes)
    game->cells = malloc(GRID_SIZE * sizeof(*game->cells));
    if (game->cells == NULL) {
        return false;
    }
    for (int row = 0; row < GRID_SIZE; ++row) {
        //calloc (et pas malloc) car ca met tout a 0 -> la grille demarre vide
        game->cells[row] = calloc(GRID_SIZE, sizeof(**game->cells));
        if (game->cells[row] == NULL) {
            //si ca rate je libere les lignes deja alloué (sinon fuite memoire)
            for (int i = 0; i < row; ++i) {
                free(game->cells[i]);
            }
            free(game->cells);
            game->cells = NULL;
            return false;
        }
    }
    return true;
}

//libere la grille alloué par game_init (pour rendre la memoire)
void game_destroy(Game *game) {
    if (game->cells == NULL) {
        return;
    }
    //je libere dans l'ordre inverse : d'abord chaque ligne, puis le tableau principal
    for (int row = 0; row < GRID_SIZE; ++row) {
        free(game->cells[row]);
    }
    free(game->cells);
    game->cells = NULL; //a NULL pour eviter un double free
}
