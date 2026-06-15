# 2048 en C / SDL2

Un jeu **2048** écrit en **C (standard C23)** avec la bibliothèque graphique **SDL2**.
Le code est découpé en plusieurs modules pour la clarté.

```
game.c / game.h          -> types partagés (Game, MoveDirection, GRID_SIZE) + création/destruction du jeu (mémoire)
logic.c / logic.h        -> règles du jeu (déplacements, fusions, victoire/défaite)
render.c / render.h      -> affichage à l'écran (SDL2)
animation.c / animation.h-> animations des tuiles (apparition + fusion)
main.c                   -> initialisation SDL, boucle principale, clavier
```

## Compilation et lancement

Le projet utilise CMake. SDL2 est récupéré automatiquement s'il n'est pas trouvé sur le système.

```sh
cmake -B cmake-build-debug
cmake --build cmake-build-debug --target 2048
```

Puis lancer l'exécutable :

```sh
./cmake-build-debug/2048        # Linux / macOS
.\cmake-build-debug\2048.exe    # Windows
```

## Commandes

- **Déplacement** : flèches, ZQSD (clavier français) ou WASD (clavier anglais)
- **R** / **N** : nouvelle partie
- **Échap** : quitter

---

## 1. `game.h` / `game.c` — Le contrat commun + la mémoire du jeu

`game.h` est le fichier inclus par tous les autres : il définit les **types de base** du jeu et
déclare les fonctions de création/destruction. `game.c` contient leur **implémentation** (l'allocation
mémoire de la grille, voir la section 2).

```c
#define GRID_SIZE 4
```
La grille fait 4×4. On utilise une constante : si on voulait une grille 5×5, on ne change qu'ici.

```c
typedef enum {
    MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN
} MoveDirection;
```
Une **énumération** : les 4 directions possibles. Plus lisible que d'utiliser des nombres 0/1/2/3.

```c
typedef struct {
    int **cells;     // la grille 4x4 (tableau 2D dynamique)
    int score;       // score courant
    bool won;        // true si on a atteint 2048
    bool game_over;  // true si plus aucun coup possible
} Game;
```
Une **structure** qui regroupe tout l'état du jeu dans un seul objet. On passe ensuite un `Game *` aux fonctions au lieu de trimballer plein de variables séparées.

---

## 2. `game.c` — L'allocation mémoire

La grille est un **tableau 2D alloué dynamiquement** (`int **cells`), construit en **deux étages** :
- étage 1 : un tableau de 4 pointeurs (les lignes)
- étage 2 : chaque ligne pointe vers un tableau de 4 `int` (les colonnes)

### `game_init` — réserver la mémoire

```c
game->cells = malloc(GRID_SIZE * sizeof(*game->cells));
if (game->cells == NULL) return false;
```
On alloue d'abord le tableau des 4 lignes. On **vérifie toujours** le retour de `malloc` : s'il vaut `NULL`, l'allocation a échoué.

```c
for (int row = 0; row < GRID_SIZE; ++row) {
    game->cells[row] = calloc(GRID_SIZE, sizeof(**game->cells));
    if (game->cells[row] == NULL) {
        for (int i = 0; i < row; ++i) free(game->cells[i]);  // on libère ce qui a déjà été alloué
        free(game->cells);
        game->cells = NULL;
        return false;
    }
}
return true;
```
Pour chaque ligne, on alloue 4 entiers avec **`calloc`** (et pas `malloc`) car `calloc` **met la mémoire à zéro** → la grille démarre vide.
Si une allocation échoue en cours de route, on **libère tout ce qui a déjà été alloué** avant de retourner `false` (pas de fuite mémoire).

### `game_destroy` — libérer la mémoire

```c
for (int row = 0; row < GRID_SIZE; ++row) free(game->cells[row]); // chaque ligne
free(game->cells);                                                // puis le tableau de lignes
game->cells = NULL;
```
On libère dans **l'ordre inverse** de l'allocation : d'abord les lignes, puis le tableau des pointeurs.
`cells = NULL` à la fin évite un **double-free** ou un pointeur invalide.

> **Règles d'or :** un `free` pour chaque `malloc`/`calloc` · toujours tester `== NULL` · libérer dans l'ordre inverse.

---

## 3. `logic.c` — Les règles du jeu

### `add_random_tile` — poser une tuile au hasard

On parcourt la grille, on liste toutes les cases **vides** (valeur 0), puis on en choisit une au hasard.

```c
int selected = rand() % empty_count;
game->cells[...] = (rand() % 10 == 0) ? 4 : 2;
```
On y pose un **2** (9 fois sur 10) ou un **4** (1 fois sur 10) — comme dans le vrai 2048.

### `process_line` — le cœur de l'algorithme (compactage + fusion)

C'est la fonction la plus importante. Elle traite **une seule ligne/colonne** ramenée vers la gauche :

1. **Compactage** : on enlève les zéros (on tasse les nombres vers la gauche).
   `[2,0,2,0]` → `[2,2]`
2. **Fusion** : deux tuiles identiques voisines fusionnent en une (×2), et on ajoute la valeur au score.
   `[2,2]` → `[4]`, et `score += 4`.
   Important : une tuile ne fusionne **qu'une seule fois** par coup (le `++i` saute la tuile déjà fusionnée).
3. On réécrit la ligne et on renvoie `true` si quelque chose a changé.

### `move_tiles` — appliquer un déplacement à toute la grille

Astuce de conception : **au lieu d'écrire 4 algorithmes** (gauche/droite/haut/bas), on **réutilise `process_line`** pour les 4 directions.

```c
if (direction == MOVE_LEFT || direction == MOVE_RIGHT) {
    row = index;
    column = direction == MOVE_LEFT ? position : GRID_SIZE - 1 - position;
} else {
    row = direction == MOVE_UP ? position : GRID_SIZE - 1 - position;
    column = index;
}
```
On **extrait** chaque ligne/colonne dans un tableau temporaire `line[]`, dans le bon ordre selon la direction (inversé pour DROITE et BAS), on appelle `process_line`, puis on **réécrit** au même endroit.

Après un coup qui change la grille :
- on ajoute une nouvelle tuile (`add_random_tile`),
- on vérifie si une case atteint 2048 (→ `won = true`),
- on vérifie s'il reste un coup possible (`can_move`), sinon `game_over = true`.

### `can_move` — reste-t-il un coup possible ?

Renvoie `true` s'il existe une case vide **ou** deux cases voisines identiques (donc fusionnables). Sinon la partie est finie.

### `reset_game` — (re)démarrer une partie

Remet toutes les cases à 0, le score à 0, les états à `false`, puis pose **2 tuiles** de départ.

---

## 4. `render.c` — L'affichage (SDL2)

Tout le dessin se fait ici. On n'utilise **pas de police de caractères externe** : les chiffres sont dessinés « à la main ».

### `Color` et les fonctions utilitaires
```c
typedef struct { Uint8 r, g, b; } Color;
```
Petite structure couleur (rouge/vert/bleu). `set_color` et `fill_rect` simplifient le dessin de rectangles colorés.

### `DIGITS` + `draw_digit` — une mini-police « maison »
```c
static const uint8_t DIGITS[10][5] = { {0x7,0x5,0x5,0x5,0x7}, ... };
```
Chaque chiffre (0–9) est dessiné dans une grille de **3 colonnes × 5 lignes**. Chaque ligne est codée sur 3 bits.
Exemple pour le `0` : `0x7 = 111`, `0x5 = 101`... ce qui dessine :
```
###
# #
# #
# #
###
```
`draw_digit` lit ces bits et dessine un petit carré (pixel) pour chaque bit à 1.

### `digit_count` et `draw_number_centered`
- `digit_count` compte le nombre de chiffres d'un nombre.
- `draw_number_centered` calcule automatiquement la **taille** (scale) et la **position** pour centrer un nombre dans un rectangle donné. La boucle réduit le scale tant que le nombre dépasse du cadre.

### `tile_color`
Renvoie la couleur d'une tuile selon sa valeur (2 = beige, 4 = beige plus foncé, ... 2048+ = jaune doré), comme le vrai jeu.

### `draw_game`
La fonction publique qui dessine **toute** la frame, dans l'ordre :
1. le fond,
2. le titre « 2048 » et le score,
3. le plateau et chaque tuile (couleur + nombre),
4. un **overlay** semi-transparent si la partie est gagnée ou perdue,
5. `SDL_RenderPresent` → affiche le tout à l'écran.

---

## 5. `main.c` — Le point d'entrée

### `direction_from_key`
Traduit une touche clavier en `MoveDirection`. Gère **3 schémas** de touches : les flèches, le **ZQSD** (clavier français) et le **WASD** (clavier anglais).

### `main`
Déroulé classique d'un programme SDL :
1. `srand(time(NULL))` → initialise le générateur aléatoire (sinon les tuiles seraient identiques à chaque lancement).
2. `SDL_Init` → initialise SDL. **Vérifié** (message d'erreur + sortie si échec).
3. Création de la **fenêtre** et du **renderer**. Si le renderer accéléré échoue, on bascule sur le rendu logiciel (`SDL_RENDERER_SOFTWARE`) — robustesse.
4. `game_init` → alloue la grille (avec gestion d'erreur), `reset_game` → lance la partie, `animation_init` → prépare les animations.
5. **Boucle principale** (`while running`) :
   - `SDL_PollEvent` lit les événements (fermeture, touches),
   - `Échap` quitte, `R`/`N` recommence, une flèche/ZQSD/WASD joue un coup,
   - on calcule `dt` (temps écoulé via `SDL_GetTicks`), puis `animation_observe` + `animation_update` font vivre les animations,
   - `draw_game` redessine, `SDL_Delay(1)` évite d'utiliser 100 % du CPU.
6. À la fin : `game_destroy` (libère la mémoire) puis on détruit renderer/fenêtre et `SDL_Quit`.

> Remarque : **chaque ressource créée est libérée**, et dans le bon ordre (renderer → fenêtre → SDL). En cas d'erreur en cours d'init, on nettoie aussi ce qui a déjà été créé.

---

## 6. `animation.c` — Les animations des tuiles

Ce module ajoute deux petits effets : une tuile qui **apparaît** (elle grossit de 0 jusqu'à sa taille
avec un léger rebond) et une **fusion** (un petit « pop » : la tuile grossit puis revient).

L'idée maligne : au lieu de modifier la logique du jeu, le module **compare la grille à la frame
précédente** pour deviner ce qui a changé. Du coup `logic.c` n'est **pas touché**.

- `animation_observe` : compare la grille actuelle à la précédente.
  - une case qui passe de `0` à une valeur → animation **d'apparition** (`ANIM_SPAWN`),
  - une case dont la valeur change alors qu'il y avait déjà une tuile → animation de **fusion** (`ANIM_MERGE`).
- `animation_update(dt)` : fait avancer le minuteur de chaque animation et la coupe quand elle est finie.
  `dt` est le temps écoulé depuis la frame précédente (calculé dans `main.c` avec `SDL_GetTicks`).
- `animation_scale` : renvoie le **facteur de taille** à appliquer à une tuile (1.0 = taille normale,
  plus petit pendant l'apparition, un peu plus gros pendant le pop).

Côté affichage, `draw_game` dessine toujours le fond de la case puis la tuile **redimensionnée** selon
`animation_scale`, recentrée pour qu'elle grossisse depuis le milieu.

> Avantage de ce découpage : les animations sont **isolées** dans leur propre module et ne dépendent
> que de l'état du jeu — on pourrait les enlever sans rien casser.

---

## Schéma mémoire de la grille (`int **cells`)

```
cells ──► [ ligne0 ]──► [ 0 | 2 | 0 | 4 ]
          [ ligne1 ]──► [ 2 | 0 | 0 | 0 ]
          [ ligne2 ]──► [ 0 | 0 | 8 | 0 ]
          [ ligne3 ]──► [ 0 | 0 | 0 | 2 ]
           (4 pointeurs)   (4 int chacun)
```
