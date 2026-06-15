#include "animation.h"

//combien de temps dure chaque animation (en secondes). plus c'est petit plus c'est rapide
#define SPAWN_DURATION 0.13f
#define MERGE_DURATION 0.13f

//remet tout a zero : aucune anim en cours et la grille d'avant toute vide
void animation_init(AnimationSystem *anim) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            anim->prev[row][column] = 0;
            anim->timer[row][column] = 0.0f;
            anim->type[row][column] = ANIM_NONE;
        }
    }
}

//je compare la grille actuelle avec celle d'avant pour declencher les bonnes animation
void animation_observe(AnimationSystem *anim, const Game *game) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            int cur = game->cells[row][column];   //la valeur maintenant
            int old = anim->prev[row][column];     //la valeur a la frame d'avant

            if (cur != 0 && old == 0) {
                //la case etait vide et maintenant y'a une tuile -> elle apparait
                anim->type[row][column] = ANIM_SPAWN;
                anim->timer[row][column] = 0.0f;
            } else if (cur != 0 && old != 0 && cur != old) {
                //la valeur a changer alors qu'il y avait deja une tuile -> c'est une fusion
                anim->type[row][column] = ANIM_MERGE;
                anim->timer[row][column] = 0.0f;
            }

            anim->prev[row][column] = cur; //je met a jour la grille d'avant pour la prochaine fois
        }
    }
}

//je fait avancer le temps de chaque anim, et quand c'est fini je la coupe
void animation_update(AnimationSystem *anim, float dt) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            if (anim->type[row][column] == ANIM_NONE) {
                continue; //rien a animer sur cette case
            }
            anim->timer[row][column] += dt;

            //je regarde si l'anim est terminer selon sa duree
            float duration = anim->type[row][column] == ANIM_SPAWN ? SPAWN_DURATION : MERGE_DURATION;
            if (anim->timer[row][column] >= duration) {
                anim->type[row][column] = ANIM_NONE; //anim finie, la tuile redevient normale
                anim->timer[row][column] = 0.0f;
            }
        }
    }
}

//renvoi la taille a appliquer a la tuile (1.0 = normal, plus petit = en train d'apparaitre, plus gros = pop)
float animation_scale(const AnimationSystem *anim, int row, int column) {
    AnimType type = anim->type[row][column];
    if (type == ANIM_NONE) {
        return 1.0f; //pas d'anim -> taille normale
    }

    if (type == ANIM_SPAWN) {
        //t va de 0 a 1 pendant l'anim
        float t = anim->timer[row][column] / SPAWN_DURATION;
        if (t > 1.0f) t = 1.0f;
        //formule "ease out back" : la tuile grossit de 0 a 1 avec un petit rebond a la fin
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float u = t - 1.0f;
        return 1.0f + c3 * u * u * u + c1 * u * u;
    }

    //ANIM_MERGE : petit pop, la tuile grossit jusqu'a 1.2 au milieu puis revient a 1.0
    float t = anim->timer[row][column] / MERGE_DURATION;
    if (t > 1.0f) t = 1.0f;
    float bump = 1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f); //parabole : 0 au debut/fin, 1 au milieu
    return 1.0f + 0.2f * bump;
}
