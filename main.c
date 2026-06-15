#include <SDL.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "logic.h"
#include "render.h"
#include "animation.h"

//cette fonction transforme une touche du clavier en direction de jeu
//jai mis plusieur touche pour chaque sens : les fleches, le ZQSD (clavier fr) et le WASD (clavier us)
static bool direction_from_key(SDL_Keycode key, MoveDirection *direction) {
    switch (key) {
        case SDLK_LEFT:
        case SDLK_a:
        case SDLK_q:
            *direction = MOVE_LEFT;
            return true; //je renvoi true pour dire que la touche est une touche de deplacement
        case SDLK_RIGHT:
        case SDLK_d:
            *direction = MOVE_RIGHT;
            return true;
        case SDLK_UP:
        case SDLK_w:
        case SDLK_z:
            *direction = MOVE_UP;
            return true;
        case SDLK_DOWN:
        case SDLK_s:
            *direction = MOVE_DOWN;
            return true;
        default:
            return false; //la touche sert a rien pour le jeu donc je renvoi false
    }
}

int main(int argc, char *argv[]) {
    (void)argc; //je met ca juste pour pas avoir de warning car je me sert pas de argc/argv
    (void)argv;
    srand((unsigned int)time(NULL)); //jinitialise le hasard avec l'heure sinon ca sort toujour pareil

    //jinitialise SDL2 (la partie video), si ca marche pas jaffiche une erreur et je quitte
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Impossible d'initialiser SDL2 : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    //je cree la fenetre du jeu avec un titre et la taille que jai definit dans render.h
    SDL_Window *window = SDL_CreateWindow(
        "2048 - SDL2 | Fleches/ZQSD/WASD, R: recommencer, Echap: quitter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) { //si la fenetre se cree pas je nettoie SDL et je quitte
        fprintf(stderr, "Impossible de creer la fenetre : %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    //le renderer c'est ce qui sert a dessiner. jessaye d'abord la version acceleré (carte graphique)
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        //si ca marche pas je tente la version logiciel (plus lente mais ca marche partout)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) { //la si vraiment ca veut pas je quitte en nettoyant la fenetre
        fprintf(stderr, "Impossible de creer le renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    Game game;
    //jalloue la grille en memoire, si l'allocation rate je nettoie tout et je quitte
    if (!game_init(&game)) {
        fprintf(stderr, "Impossible d'allouer la grille de jeu\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    reset_game(&game);   //je lance une nouvelle partie

    AnimationSystem anim;
    animation_init(&anim); //je prepare le systeme d'animation (les tuiles de depart vont apparaitre)

    bool running = true; //tant que running est vrai le jeu tourne
    Uint32 last_ticks = SDL_GetTicks(); //l'heure de la frame d'avant, pour calculer le temps ecoulé

    //ca c'est la boucle principale du jeu, elle tourne en boucle jusqua qu'on quitte
    while (running) {
        SDL_Event event;
        //je regarde tout les evenement (clavier, fermeture de la fenetre...)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false; //la croix de la fenetre -> on arrete
            } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                //repeat==0 ca evite que ca rejoue plein de fois si on garde la touche enfoncer
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    running = false; //echap -> on quitte
                } else if (key == SDLK_r || key == SDLK_n) {
                    reset_game(&game);     //R ou N -> on recommence une partie
                    animation_init(&anim); //je remet les anim a zero pour que les 2 nouvelle tuiles apparaissent
                } else if (!game.game_over && !game.won) {
                    //on joue seulement si la partie est pas fini (ni gagner ni perdu)
                    MoveDirection direction;
                    if (direction_from_key(key, &direction)) {
                        move_tiles(&game, direction); //je fait bouger les tuiles
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                //je recupere la position du clic de la souris
                int mx = event.button.x;
                int my = event.button.y;

                //est-ce que le clic est dans le bouton "rejouer" du haut (toujours visible) ?
                bool clic_reset = mx >= RESET_BTN_X && mx <= RESET_BTN_X + RESET_BTN_W &&
                                  my >= RESET_BTN_Y && my <= RESET_BTN_Y + RESET_BTN_H;
                //est-ce que le clic est dans le bouton de fin (seulement si la partie est finie) ?
                bool clic_fin = (game.game_over || game.won) &&
                                mx >= END_BTN_X && mx <= END_BTN_X + END_BTN_W &&
                                my >= END_BTN_Y && my <= END_BTN_Y + END_BTN_H;

                if (clic_reset || clic_fin) {
                    reset_game(&game);     //on recommence une partie
                    animation_init(&anim); //et on remet les anim a zero
                }
            }
        }

        //je calcule dt = le temps (en secondes) ecoulé depuis la frame d'avant
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        animation_observe(&anim, &game); //je regarde ce qui a changer pour lancer les anim
        animation_update(&anim, dt);     //je fait avancer les anim en cours

        draw_game(renderer, &game, &anim); //je redessine l'ecran a chaque tour de boucle
        SDL_Delay(1); //petite pause pour pas que le programme prenne 100% du processeur
    }

    //a la fin je libere tout dans le bon ordre pour pas faire de fuite memoire
    game_destroy(&game);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
