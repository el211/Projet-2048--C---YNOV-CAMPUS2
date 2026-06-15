#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>

#include "game.h"

//remet la partie a zero : grille vider, score et etats remis a 0, puis pose 2 tuiles de depart
//attention la grille doit deja etre alloué avant
void reset_game(Game *game);

//fait un deplacement dans la direction donner. si le plateau change ca ajoute une tuile
//au hasard et ca met a jour won/game_over. renvoi true si le plateau a changer
bool move_tiles(Game *game, MoveDirection direction);

#endif /* LOGIC_H */
