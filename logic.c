#include "logic.h"

#include <stdlib.h>

//cette fonction sert a poser une nouvelle tuile (2 ou 4) sur une case vide au hasard
static void add_random_tile(Game *game) {
    int empty[GRID_SIZE * GRID_SIZE][2]; //je fait un tableau pour stocker les cases vides (la ligne et la colone)
    int empty_count = 0; //compteur du nombre de case vide trouver

    //la je parcour toute la grille avec 2 boucle pour trouver les cases vide (qui valent 0)
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            if (game->cells[row][column] == 0) {
                empty[empty_count][0] = row;    //je retiens la ligne
                empty[empty_count][1] = column; //et la colone
                ++empty_count;
            }
        }
    }

    if (empty_count == 0) {
        return; //si y'a plus aucune case vide on fait rien et on sort
    }

    int selected = rand() % empty_count; //je choisi une case vide au hasard parmis celle trouver
    //rand()%10==0 ca donne 1 chance sur 10, donc 10% de chance d'avoir un 4 sinon un 2
    game->cells[empty[selected][0]][empty[selected][1]] = (rand() % 10 == 0) ? 4 : 2;
}

//cette fonction verifie si on peut encore jouer (sinon c'est game over)
static bool can_move(const Game *game) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            if (game->cells[row][column] == 0) {
                return true; //y'a une case vide donc on peut encore bouger
            }
            //je regarde si la case a droite est pareil (donc fusion possible)
            if (column + 1 < GRID_SIZE &&
                game->cells[row][column] == game->cells[row][column + 1]) {
                return true;
            }
            //je regarde si la case en dessous est pareil (fusion possible aussi)
            if (row + 1 < GRID_SIZE &&
                game->cells[row][column] == game->cells[row + 1][column]) {
                return true;
            }
        }
    }
    return false; //aucune case vide et aucune fusion possible -> on peut plus jouer
}

//ca c'est la fonction la plus importante : elle traite UNE ligne et la pousse vers la gauche
//elle fait le tassage des chiffre + les fusion. elle renvoi true si la ligne a changer
static bool process_line(int line[GRID_SIZE], int *score) {
    int original[GRID_SIZE];          //je garde la ligne de depart pour comparer a la fin
    int compacted[GRID_SIZE] = {0};   //la ligne sans les zero (tasser a gauche)
    int output[GRID_SIZE] = {0};      //le resultat final apres les fusion
    int compacted_count = 0;
    int output_count = 0;

    //etape 1 : je vire les 0 et je tasse les chiffre a gauche
    for (int i = 0; i < GRID_SIZE; ++i) {
        original[i] = line[i];
        if (line[i] != 0) {
            compacted[compacted_count++] = line[i];
        }
    }

    //etape 2 : je fusionne les chiffre identique qui sont cote a cote
    for (int i = 0; i < compacted_count; ++i) {
        if (i + 1 < compacted_count && compacted[i] == compacted[i + 1]) {
            output[output_count] = compacted[i] * 2; //2 cases pareil -> ca fait le double
            *score += output[output_count];          //jajoute la valeur au score
            ++output_count;
            ++i; //je saute la case suivante pour pas la refusionner 2 fois
        } else {
            output[output_count++] = compacted[i]; //pas de fusion, je recopie tel quel
        }
    }

    //etape 3 : je remet le resultat dans la ligne et je verifie si quelque chose a bouger
    bool changed = false;
    for (int i = 0; i < GRID_SIZE; ++i) {
        line[i] = output[i];
        if (line[i] != original[i]) {
            changed = true;
        }
    }
    return changed;
}

//cette fonction applique le deplacement sur toute la grille selon la direction choisi
bool move_tiles(Game *game, MoveDirection direction) {
    bool changed = false;

    // au lieu de faire 4 code different (gauche/droite/haut/bas)
    //je reutilise process_line pour les 4 sens en lisant la grille dans le bon ordre
    for (int index = 0; index < GRID_SIZE; ++index) {
        int line[GRID_SIZE];
        //je copie une ligne (ou colone) dans un petit tableau temporaire line[]
        for (int position = 0; position < GRID_SIZE; ++position) {
            int row;
            int column;
            if (direction == MOVE_LEFT || direction == MOVE_RIGHT) {
                row = index;
                //pour la droite je lis a l'envers (GRID_SIZE-1-position) comme ca ca pousse a droite
                column = direction == MOVE_LEFT ? position : GRID_SIZE - 1 - position;
            } else {
                //pareil pour haut/bas mais cette fois c'est les ligne qu'on inverse
                row = direction == MOVE_UP ? position : GRID_SIZE - 1 - position;
                column = index;
            }
            line[position] = game->cells[row][column];
        }

        if (process_line(line, &game->score)) {
            changed = true; //si au moins une ligne change alors la grille a changer
        }

        //je remet la ligne traiter au meme endroit (dans le meme ordre que quand je l'ai lu)
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

    //si la grille a bouger alors je fait les choses d'apres un coup
    if (changed) {
        add_random_tile(game); //on ajoute une nouvelle tuile
        //je regarde si une case a atteint 2048 -> alors le joueur a gagner
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int column = 0; column < GRID_SIZE; ++column) {
                if (game->cells[row][column] >= 2048) {
                    game->won = true;
                }
            }
        }
        game->game_over = !can_move(game); //si on peut plus bouger alors c'est perdu
    }
    return changed;
}

//cette fonction remet la partie a zero pour recommencer
void reset_game(Game *game) {
    //je remet toute les case a 0 (grille vide)
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            game->cells[row][column] = 0;
        }
    }
    game->score = 0;          //le score repart de 0
    game->won = false;        //pas encore gagner
    game->game_over = false;  //pas encore perdu
    add_random_tile(game);    //je met 2 tuiles de depart comme dans le vrai jeu
    add_random_tile(game);
}
