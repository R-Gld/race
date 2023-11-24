#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

#define BUFSIZE 256
#define INT_MIN (-2147483648)

/**
 * @brief Affiche un message de débogage.
 * @param format Format du message.
 * @param ... Arguments du message.
 * @note Cette fonction n'est pas utilisée dans le code du jeu.
 */
void debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, "DEBUG: ", args);
    vfprintf(stderr, format, args);
    vfprintf(stderr, "\n", args);
}


/**
 * @brief Structure représentant la grille du jeu.
 *
 * @param size Taille de la grille (nombre de cases sur une ligne/colonne).
 * @param values Tableau dynamique contenant les valeurs de chaque case de la grille.
 *               Chaque valeur peut représenter un bonus ou un malus.
 */
struct grid {
    int size;
    int *values;
};

/**
 * @brief Structure représentant le joueur dans le jeu.
 *
 * @param x Position actuelle du joueur sur l'axe X de la grille.
 * @param y Position actuelle du joueur sur l'axe Y de la grille.
 * @param vx Vitesse actuelle du joueur sur l'axe X.
 * @param vy Vitesse actuelle du joueur sur l'axe Y.
 */
struct player {
    int x, y;
    int vx, vy;
};

/**
 * @brief Structure représentant une zone d'objectif dans le jeu.
 *
 * @param x Position X du coin supérieur gauche de la zone d'objectif.
 * @param y Position Y du coin supérieur gauche de la zone d'objectif.
 * @param w Largeur de la zone d'objectif.
 * @param h Hauteur de la zone d'objectif.
 */
struct objective_area {
    int x, y;
    int w, h;
};

/**
 * @brief Structure représentant un point spécifique comme objectif dans la grille.
 *
 * @param x Position X du point objectif dans la grille.
 * @param y Position Y du point objectif dans la grille.
 */
struct objective_point {
    int x, y;
};

/**
 * @brief Structure principale représentant l'état actuel du jeu.
 *
 * @param grid Pointeur vers la structure grid représentant la grille du jeu.
 * @param player Pointeur vers la structure player représentant le joueur.
 */
struct game {
    struct grid *grid;
    struct player *player;
};


struct game *init_game(char buf[BUFSIZE], bool DEBUG);
void end_game(struct game *self, struct objective_area *objective);

struct grid *grid_create(char buf[BUFSIZE], bool DEBUG);
int grid_access_value(struct grid *self, size_t r, size_t c);
void grid_destroy(struct grid *self);
void grid_set_value(struct grid *self, size_t r, size_t c, int value);

struct player *player_create(char buf[BUFSIZE], bool DEBUG);
void player_destroy(struct player *self);

void update_velocity_towards_objective(struct player *p, struct objective_point *obj);

struct objective_area *objective_area_create(char buf[BUFSIZE], bool DEBUG);
void objective_area_destroy(struct objective_area *self);

struct objective_point *choose_objective_point(struct grid *grid, struct objective_area *objective);

bool check_serv(char buf[BUFSIZE], char *server_answer);



/**
 * @brief Point d'entrée principal du jeu "Race".
 *
 * Cette fonction initialise le jeu, gère la boucle de jeu principale,
 * et nettoie les ressources à la fin. Elle interagit avec le serveur pour
 * obtenir des informations sur la grille, le joueur et les objectifs, puis
 * calcule les déplacements du joueur en fonction de ces données.
 *
 * @param argc Nombre d'arguments passés au programme.
 * @param argv Tableau de chaînes représentant les arguments passés.
 * @return Code de sortie du programme. EXIT_SUCCESS en cas de succès, sinon un code d'erreur.
 */
int main(int argc, char **argv) {
    bool DEBUG = false;
    if(argc == 2) {
        if(strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0) {
            DEBUG = true;
            debug("Debug mode activated");
        }
    }

    setbuf(stdout, NULL);
    char buf[BUFSIZE];


    struct game *game = init_game(buf, DEBUG);

    // get the initial objective area
    struct objective_area *objective_area = objective_area_create(buf, DEBUG);
    struct objective_point *real_objective = choose_objective_point(game->grid, objective_area);

    while (true) {
        update_velocity_towards_objective(game->player, real_objective);


        // Compute new position based on current velocity
        int new_x = game->player->x + game->player->vx;
        int new_y = game->player->y + game->player->vy;


        // Ensure the new position is within the grid bounds
        if (new_x < 0 || new_x >= game->grid->size || new_y < 0 || new_y >= game->grid->size) {
            fprintf(stderr, "Invalid move: out of bounds\n");
            break;
        }

        // Update the player's position
        game->player->x = new_x;
        game->player->y = new_y;

        printf("%i\n%i\n", game->player->x, game->player->y);

        // Get the server's response
        if (fgets(buf, BUFSIZE, stdin) == NULL) {
            perror("Error reading from server");
            break;
        }

        if (check_serv(buf, "ERROR\n")) {
            fprintf(stderr, "Invalid move\n");
            break;
        } else if (check_serv(buf, "FINISH\n")) {
            printf("Game finished successfully\n");
            break;
        } else if (check_serv(buf, "CHECKPOINT\n")) {
            // Handle the new objective
            objective_area_destroy(objective_area);
            objective_area = objective_area_create(buf, DEBUG);
            real_objective = choose_objective_point(game->grid, objective_area);
        } else if (!check_serv(buf, "OK\n")) {
            fprintf(stderr, "Unexpected server response: %s\n", buf);
            break;
        }

        // Here, implement the logic to decide the next move and update the player's velocity

    }

    end_game(game, objective_area);
    return EXIT_SUCCESS;
}

/**
 * @brief Met à jour la vitesse du joueur pour se déplacer vers un point objectif.
 * @param p Pointeur vers le joueur.
 * @param obj Pointeur vers le point objectif.
 */
void update_velocity_towards_objective(struct player *p, struct objective_point *obj) {
    // Simple logic to move towards the objective point
    if (p->x < obj->x) {
        p->vx = 1;  // Move right
    } else if (p->x > obj->x) {
        p->vx = -1; // Move left
    } else {
        p->vx = 0;  // Don't move horizontally
    }

    if (p->y < obj->y) {
        p->vy = 1;  // Move down
    } else if (p->y > obj->y) {
        p->vy = -1; // Move up
    } else {
        p->vy = 0;  // Don't move vertically
    }
}

/**
 * @brief Sélectionne le meilleur point dans la zone d'objectif en fonction des valeurs de la grille.
 * @param grid Pointeur vers la grille.
 * @param objective Pointeur vers la zone d'objectif.
 * @return Pointeur vers le point objectif optimal choisi.
 */
struct objective_point *choose_objective_point(struct grid *grid, struct objective_area *objective) {
    struct objective_point *optimal_point = malloc(sizeof(struct objective_point));
    int max_value = INT_MIN;

    // Parcourir chaque case dans la zone de l'objectif
    for (int i = objective->x; i < objective->x + objective->w; ++i) {
        for (int j = objective->y; j < objective->y + objective->h; ++j) {
            // Vérifier si la case est dans les limites de la grille
            if (i >= 0 && i < grid->size && j >= 0 && j < grid->size) {
                int current_value = grid_access_value(grid, i, j);
                // Choisir le point avec la valeur la plus élevée
                if (current_value > max_value) {
                    max_value = current_value;
                    optimal_point->x = i;
                    optimal_point->y = j;
                }
            }
        }
    }
    return optimal_point;
}

/**
 * @brief Vérifie si la réponse du serveur correspond à une chaîne donnée.
 * @param buf Tampon contenant la réponse du serveur.
 * @param server_answer Chaîne de caractères à comparer avec la réponse du serveur.
 * @return Booléen indiquant si la réponse du serveur correspond à la chaîne donnée.
 */
bool check_serv(char buf[BUFSIZE], char *server_answer) {
    return strcmp(buf, server_answer) == 0;
}

/**
 * @brief Initialise le jeu en créant et en configurant les structures de données nécessaires.
 * @param buf Un tampon pour lire les entrées.
 * @param DEBUG Un booléen pour activer ou désactiver le mode débogage.
 * @return Pointeur vers une structure 'game' nouvellement allouée et initialisée.
 */
struct game *init_game(char buf[BUFSIZE], bool DEBUG) {
    // get the grid
    struct grid *grid = grid_create(buf, DEBUG);

    // get the player
    struct player *player = player_create(buf, DEBUG);

    struct game *game = malloc(sizeof(struct game));
    game->grid = grid;
    game->player = player;
    return game;
}

/**
 * @brief Termine le jeu en libérant toutes les ressources allouées.
 * @param self Pointeur vers la structure de jeu à nettoyer.
 * @param objective Pointeur vers la zone d'objectif à libérer.
 */
void end_game(struct game *self, struct objective_area *objective) {
    grid_destroy(self->grid);
    player_destroy(self->player);
    objective_area_destroy(objective);
    free(self);
}

/**
 * @brief Crée une grille pour le jeu à partir des données lues.
 * @param buf Un tampon pour lire les valeurs de la grille.
 * @param DEBUG Un booléen pour activer ou désactiver le mode débogage.
 * @return Pointeur vers une grille nouvellement allouée et remplie.
 */
struct grid *grid_create(char buf[256], bool DEBUG) {
    struct grid *grid = malloc(sizeof(struct grid));

    if(DEBUG) debug("grid size ?");
    // get the size of the grid
    fgets(buf, BUFSIZE, stdin);
    int size = atoi(buf);
    grid->size = size;

    grid->values = calloc(size*size, sizeof(int));


    // add the value of the grid
    for (int i = 0; i < size; ++i) {
        for(int j = 0; j < size; ++j) {
            if(DEBUG) debug("value of the grid à i: %i, j: %i ?", i, j);

            fgets(buf, BUFSIZE, stdin);
            int value = atoi(buf);
            if(DEBUG) debug("value of the grid à i: %i, j: %i = %i", i, j, value);
            grid_set_value(grid, i, j, value);
        }
    }
    if(DEBUG) debug("grid created");
    return grid;
}

/**
 * @brief Libère la mémoire allouée pour la grille.
 * @param self Pointeur vers la grille à détruire.
 */
void grid_destroy(struct grid *self) {
    assert(self != NULL);
    self->size = 0;
    free(self->values);
    free(self);
}

/**
 * @brief Accède à la valeur dans la grille à un emplacement donné.
 * @param self Pointeur vers la grille.
 * @param r Indice de la ligne.
 * @param c Indice de la colonne.
 * @return Valeur à la position (r, c) dans la grille.
 */
int grid_access_value(struct grid *self, size_t r, size_t c) {
    assert(r < self->size);
    assert(c < self->size);
    return self->values[r*self->size + c];
}

/**
 * @brief Définit une valeur dans la grille à un emplacement donné.
 * @param self Pointeur vers la grille.
 * @param r Indice de la ligne.
 * @param c Indice de la colonne.
 * @param value Valeur à définir à la position (r, c).
 */
void grid_set_value(struct grid *self, size_t r, size_t c, int value) {
    assert(r < self->size);
    assert(c < self->size);
    self->values[r*self->size+c] = value;
}

/**
 * @brief Crée un joueur et initialise sa position et sa vitesse.
 * @param buf Un tampon pour lire la position initiale du joueur.
 * @param DEBUG Un booléen pour activer ou désactiver le mode débogage.
 * @return Pointeur vers un joueur nouvellement alloué et initialisé.
 */
struct player *player_create(char buf[BUFSIZE], bool DEBUG) {
    if(DEBUG) debug("player position\n x: ");
    fgets(buf, BUFSIZE, stdin);
    int x = atoi(buf);
    if(DEBUG) debug(" y: ");
    fgets(buf, BUFSIZE, stdin);
    int y = atoi(buf);
    struct player *player = malloc(sizeof(struct player));
    player->x = x;
    player->y = y;
    player->vx = 0;
    player->vy = 0;
    return player;
}

/**
 * @brief Libère la mémoire allouée pour le joueur.
 * @param self Pointeur vers le joueur à détruire.
 */
void player_destroy(struct player *self){
    assert(self != NULL);
    self->x = 0;
    self->y = 0;
    self->vx = 0;
    self->vy = 0;
    free(self);
}

/**
 * @brief Crée une zone d'objectif à partir des données lues.
 * @param buf Un tampon pour lire la position et la taille de la zone d'objectif.
 * @param DEBUG Un booléen pour activer ou désactiver le mode débogage.
 * @return Pointeur vers une zone d'objectif nouvellement allouée.
 */
struct objective_area* objective_area_create(char buf[BUFSIZE], bool DEBUG) {
    if(DEBUG) debug("objective_area position\n x: ");
    fgets(buf, BUFSIZE, stdin);
    int x = atoi(buf);
    if(DEBUG) debug(" y: ");
    fgets(buf, BUFSIZE, stdin);
    int y = atoi(buf);
    if(DEBUG) debug("objective_area size\n w: ");
    fgets(buf, BUFSIZE, stdin);
    int w = atoi(buf);
    if(DEBUG) debug(" h: ");
    fgets(buf, BUFSIZE, stdin);
    int h = atoi(buf);
    struct objective_area * objective = malloc(sizeof(struct objective_area));
    objective->x = x;
    objective->y = y;
    objective->w = w;
    objective->h = h;
    return objective;
}

/**
 * @brief Libère la mémoire allouée pour la zone d'objectif.
 * @param self Pointeur vers la zone d'objectif à détruire.
 */
void objective_area_destroy(struct objective_area *self) {
    assert(self != NULL);
    self->x = 0;
    self->y = 0;
    self->w = 0;
    self->h = 0;
    free(self);
}
