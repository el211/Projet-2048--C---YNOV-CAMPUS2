#ifndef RENDER_H
#define RENDER_H

#include "game.h"
#include "animation.h"

#define WINDOW_WIDTH 560  //largeur de la fenetre
#define WINDOW_HEIGHT 700 //hauteur de la fenetre

struct SDL_Renderer;

//dessine tout l'ecran (le titre, le score, le plateau et le voile de fin) et affiche le resultat
//anim sert a savoir de quel taille dessiner chaque tuile (pour les animation)
void draw_game(struct SDL_Renderer *renderer, const Game *game, const AnimationSystem *anim);

#endif /* RENDER_H */
