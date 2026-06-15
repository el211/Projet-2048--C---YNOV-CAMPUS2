#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

//ce fichier c'est le contrat commun, il est inclu par tout les autres fichiers
//jai mis ici les type de base que jutilise partout dans le jeu

#define GRID_SIZE 4 //la grille fait 4x4. comme c'est une constante si je veux du 5x5 je change que ici

//les 4 direction possible, c'est plus clair que d'utiliser des nombre 0 1 2 3
typedef enum {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN
} MoveDirection;


//cette structure regroupe tout l'etat du jeu dans un seul truc
//comme ca je passe juste un Game* aux fonction au lieu de plein de variable separer
typedef struct {
    int **cells;     //la grille 4x4 (tableau 2D alloué dynamiquement)
    int score;       //le score actuel
    bool won;        //true si on a atteint 2048
    bool game_over;  //true si y'a plus aucun coup possible
} Game;

//alloue la grille du jeu en memoire (tableau 2D de 4x4). renvoi false si une allocation rate
bool game_init(Game *game);

//libere la grille alloué par game_init
void game_destroy(Game *game);

#endif /* GAME_H */
