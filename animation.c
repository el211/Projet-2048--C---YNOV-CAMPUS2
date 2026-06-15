#include "animation.h"

//combien de temps dure chaque animation (en secondes). plus c'est petit plus c'est rapide
#define SPAWN_DURATION 0.13f
#define MERGE_DURATION 0.13f

//reglages du battement de coeur quand on est au repos
#define HEARTBEAT_DELAY 1.5f      //temps d'inactivité avant que ca commence (en secondes)
#define HEARTBEAT_PERIOD 1.1f     //duree d'un battement complet
#define HEARTBEAT_AMPLITUDE 0.04f //a quel point la tuile grossit (0.04 = 4%, faut rester discret)

//remet tout a zero : aucune anim en cours et la grille d'avant toute vide
void animation_init(AnimationSystem *anim) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int column = 0; column < GRID_SIZE; ++column) {
            anim->prev[row][column] = 0;
            anim->timer[row][column] = 0.0f;
            anim->type[row][column] = ANIM_NONE;
        }
    }
    anim->idle_time = 0.0f;
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
    bool busy = false; //est-ce qu'il y a au moins une anim (spawn/merge) en cours ?

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
            } else {
                busy = true; //une anim tourne encore
            }
        }
    }

    //si rien ne bouge je compte le temps d'inactivité (pour le battement de coeur)
    //sinon je remet a zero pour que le coeur reparte du calme apres un coup
    if (busy) {
        anim->idle_time = 0.0f;
    } else {
        anim->idle_time += dt;
    }
}

//petit battement de coeur : deux bosses rapprochees (lub-dub) puis du repos, comme un vrai coeur
//renvoi un nombre entre 0 (calme) et 1 (pic du battement)
static float heartbeat_pulse(float idle_time) {
    if (idle_time < HEARTBEAT_DELAY) {
        return 0.0f; //pas encore assez longtemps au repos
    }

    //je ramene le temps dans un seul battement (un modulo fait a la main, sans math.h)
    float t = idle_time - HEARTBEAT_DELAY;
    float phase = (t - HEARTBEAT_PERIOD * (int)(t / HEARTBEAT_PERIOD)) / HEARTBEAT_PERIOD; //entre 0 et 1

    //une petite fonction "bosse" : vaut 1 au centre et 0 sur les bords (parabole)
    float pulse = 0.0f;
    float centers[2] = {0.12f, 0.30f};   //les 2 coups du coeur (lub puis dub)
    float heights[2] = {1.0f, 0.6f};     //le 2eme coup est plus faible
    for (int i = 0; i < 2; ++i) {
        float x = (phase - centers[i]) / 0.09f; //0.09 = largeur de la bosse
        if (x > -1.0f && x < 1.0f) {
            float bump = (1.0f - x * x) * heights[i];
            if (bump > pulse) pulse = bump; //je garde la bosse la plus haute
        }
    }
    return pulse;
}

//renvoi la taille a appliquer a la tuile (1.0 = normal, plus petit = en train d'apparaitre, plus gros = pop)
float animation_scale(const AnimationSystem *anim, int row, int column) {
    AnimType type = anim->type[row][column];
    if (type == ANIM_NONE) {
        //pas d'anim de spawn/merge -> on applique le battement de coeur du repos
        return 1.0f + HEARTBEAT_AMPLITUDE * heartbeat_pulse(anim->idle_time);
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
