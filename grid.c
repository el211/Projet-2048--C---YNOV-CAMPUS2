#include "grid.h"

#include <stdlib.h>

//cette fonction alloue la grille en memoire (un tableau 2D de 4x4). renvoi false si une allocation rate
bool game_init(Game *game) {  //je creer une fonction qui initialise la memoire du jeu et qui recoit un pointeur vers Game
    //donc elle modifie directement la vraie structure.
    game->cells = malloc(GRID_SIZE * sizeof(*game->cells)); //j'alloues le tableau principal un tableau de pointeurs vers les lignes
    //Comme GRID_SIZE = 4, tu réserves de la mémoire pour 4 lignes.
    if (game->cells == NULL) {
        return false;
    } // je verifies si lallocation a echoué. Si oui, tu retournes false
    for (int row = 0; row < GRID_SIZE; ++row) { //je parcours les 4 lignes pour les allouer une par une
        game->cells[row] = calloc(GRID_SIZE, sizeof(**game->cells));
        //jalloue une ligne de 4 entiers. jutilise calloc et pas malloc car calloc met tout a 0
        //comme ca la grille demarre vide (que des 0) sans que jai a le faire moi meme
        if (game->cells[row] == NULL) { //si une ligne arrive pas a se creer
            for (int i = 0; i < row; ++i) {
                free(game->cells[i]); //je libere les ligne que jai deja reussi a allouer avant (sinon fuite memoire)
            }
            free(game->cells);   //je libere aussi le tableau principal
            game->cells = NULL;  //je remet le pointeur a NULL pour pas qu'il pointe dans le vide
            return false;        //et je sors en disant que ca a rater
        }
    }
    return true; //tout c'est bien passer, la grille est prete
}

//cette fonction libere la grille qui a ete alloué par game_init (pour rendre la memoire)
void game_destroy(Game *game) {
    if (game->cells == NULL) {
        return; //si y'a rien d'alloué je fait rien (evite de liberer dans le vide)
    }
    for (int row = 0; row < GRID_SIZE; ++row) {
        free(game->cells[row]); //je libere d'abord chaque ligne une par une
    }
    free(game->cells);  //ensuite je libere le tableau des pointeurs (l'ordre inverse de l'allocation)
    game->cells = NULL; //je remet a NULL pour eviter un double free ou un pointeur foireux
}
