#ifndef RENDER_H
#define RENDER_H

#include "game.h"
#include "animation.h"

#define WINDOW_WIDTH 560  //largeur de la fenetre
#define WINDOW_HEIGHT 700 //hauteur de la fenetre

//le bouton "rejouer" toujours visible en haut (sous le score)
#define RESET_BTN_X 340
#define RESET_BTN_Y 116
#define RESET_BTN_W 190
#define RESET_BTN_H 28

//le bouton "rejouer" qui apparait sur l'ecran de fin (victoire/defaite)
#define END_BTN_X 180
#define END_BTN_Y 470
#define END_BTN_W 200
#define END_BTN_H 56

struct SDL_Renderer;

//dessine tout l'ecran (le titre, le score, le plateau et le voile de fin) et affiche le resultat
//anim sert a savoir de quel taille dessiner chaque tuile (pour les animation)
void draw_game(struct SDL_Renderer *renderer, const Game *game, const AnimationSystem *anim);

#endif /* RENDER_H */
