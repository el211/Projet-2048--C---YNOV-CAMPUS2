#ifndef ANIMATION_H
#define ANIMATION_H

#include "game.h"

//ce fichier gere les petites animations des tuiles (apparition + fusion)
//l'idée : a chaque frame je compare la grille avec celle d'avant pour savoir ce qui a changer
//comme ca jai pas besoin de toucher a la logique du jeu (logic.c)

//le type d'animation en cours sur une case
typedef enum {
    ANIM_NONE,   //rien, la tuile est normale
    ANIM_SPAWN,  //une nouvelle tuile qui apparait (elle grossit de 0)
    ANIM_MERGE   //2 tuiles qui fusionnent (petit pop)
} AnimType;

//cette structure garde l'etat des animations pour toute la grille
typedef struct {
    int prev[GRID_SIZE][GRID_SIZE];        //la grille telle qu'elle etait a la frame d'avant
    float timer[GRID_SIZE][GRID_SIZE];     //depuis combien de temps l'anim de la case tourne (en secondes)
    AnimType type[GRID_SIZE][GRID_SIZE];   //le type d'anim de chaque case
    float idle_time;                       //depuis combien de temps rien ne bouge (pour le battement de coeur)
} AnimationSystem;

//remet le systeme d'anim a zero (a appeler au debut et quand on recommence une partie)
void animation_init(AnimationSystem *anim);

//compare la grille actuelle avec la precedente et declenche les animation qui vont bien
void animation_observe(AnimationSystem *anim, const Game *game);

//fait avancer le temps des animations (dt = temps ecoulé depuis la derniere frame, en secondes)
void animation_update(AnimationSystem *anim, float dt);

//renvoi le facteur de taille a appliquer a une tuile pour la dessiner (1.0 = taille normale)
float animation_scale(const AnimationSystem *anim, int row, int column);

#endif /* ANIMATION_H */
