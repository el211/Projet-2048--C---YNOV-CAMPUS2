#ifndef GRID_H
#define GRID_H

#include <stdbool.h>

#include "game.h"

//alloue la grille en memoire (tableau 2D de 4x4). renvoi false si l'allocation rate
bool game_init(Game *game);

//libere la grille qui a ete alloué par game_init
void game_destroy(Game *game);

#endif /* GRID_H */
