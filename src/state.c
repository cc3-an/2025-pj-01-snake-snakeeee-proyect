#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

// Definiciones de funciones de ayuda.
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Tarea 1 */
/* Tarea 1 */
game_state_t* create_default_state() {
  game_state_t* state = malloc(sizeof(game_state_t));
  if (state == NULL) exit(1);

  state->num_rows = 18;
  state->num_snakes = 1;

  // Crear el tablero
  state->board = malloc(sizeof(char*) * state->num_rows);
  if (state->board == NULL) exit(1);

  for (unsigned int i = 0; i < state->num_rows; i++) {
    state->board[i] = malloc(sizeof(char) * 21); // 20 + 1 por null terminator
    if (state->board[i] == NULL) exit(1);

    for (unsigned int j = 0; j < 20; j++) {
      if (i == 0 || i == 17 || j == 0 || j == 19) {
        state->board[i][j] = '#';
      } else {
        state->board[i][j] = ' ';
      }
    }
    state->board[i][20] = '\0'; // null terminator para imprimir
  }

  // Posicionar la serpiente
  state->board[2][2] = 'd';
  state->board[2][3] = '>';
  state->board[2][4] = 'D';

  // Posicionar la fruta
  state->board[2][9] = '*';

  // Crear y configurar la serpiente
  state->snakes = malloc(sizeof(snake_t));
  if (state->snakes == NULL) exit(1);

  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;

  return state;
}


/* Tarea 2 */
void free_state(game_state_t* state) {
  if (state == NULL) return;

  // Liberar cada fila del tablero
  for (unsigned int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }

  // Liberar el array de filas
  free(state->board);

  // Liberar el array de serpientes
  free(state->snakes);

  // Liberar la estructura game_state
  free(state);
}



/* Tarea 3 */
void print_board(game_state_t* state, FILE* fp) {
  for (unsigned int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s\n", state->board[i]);
  }
}



/**
 * Guarda el estado actual a un archivo. No modifica el objeto/struct state.
 * (ya implementada para que la utilicen)
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Tarea 4.1 */


/**
 * Funcion de ayuda que obtiene un caracter del tablero dado una fila y columna
 * (ya implementado para ustedes).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}


/**
 * Funcion de ayuda que actualiza un caracter del tablero dado una fila, columna y
 * un caracter.
 * (ya implementado para ustedes).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}


/**
 * Retorna true si la variable c es parte de la cola de una snake.
 * La cola de una snake consiste de los caracteres: "wasd"
 * Retorna false de lo contrario.
*/
static bool is_tail(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}



/**
 * Retorna true si la variable c es parte de la cabeza de una snake.
 * La cabeza de una snake consiste de los caracteres: "WASDx"
 * Retorna false de lo contrario.
*/
static bool is_head(char c) {
  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x';
}



/**
 * Retorna true si la variable c es parte de una snake.
 * Una snake consiste de los siguientes caracteres: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return strchr("wasd^<v>WASDx", c) != NULL;
}



/**
 * Convierte un caracter del cuerpo de una snake ("^<v>")
 * al caracter que correspondiente de la cola de una
 * snake ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
    case '^': return 'w';
    case '<': return 'a';
    case 'v': return 's';
    case '>': return 'd';
    default: return '?';
  }
}



/**
 * Convierte un caracter de la cabeza de una snake ("WASD")
 * al caracter correspondiente del cuerpo de una snake
 * ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
    case 'W': return '^';
    case 'A': return '<';
    case 'S': return 'v';
    case 'D': return '>';
    default: return '?';
  }
}



/**
 * Retorna cur_row + 1 si la variable c es 'v', 's' o 'S'.
 * Retorna cur_row - 1 si la variable c es '^', 'w' o 'W'.
 * Retorna cur_row de lo contrario
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') return cur_row + 1;
  if (c == '^' || c == 'w' || c == 'W') return cur_row - 1;
  return cur_row;
}






/**
 * Retorna cur_col + 1 si la variable c es '>' or 'd' or 'D'.
 * Retorna cur_col - 1 si la variable c es '<' or 'a' or 'A'.
 * Retorna cur_col de lo contrario
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') return cur_col + 1;
  if (c == '<' || c == 'a' || c == 'A') return cur_col - 1;
  return cur_col;
}


/**
 * Tarea 4.2
 *
 * Funcion de ayuda para update_state. Retorna el caracter de la celda
 * en donde la snake se va a mover (en el siguiente paso).
 *
 * Esta funcion no deberia modificar nada de state.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  snake_t snake = state->snakes[snum];
  char head = get_board_at(state, snake.head_row, snake.head_col);
  unsigned int next_row = get_next_row(snake.head_row, head);
  unsigned int next_col = get_next_col(snake.head_col, head);
  return get_board_at(state, next_row, next_col);
}



/**
 * Tarea 4.3
 *
 * Funcion de ayuda para update_state. Actualiza la cabeza de la snake...
 *
 * ... en el tablero: agregar un caracter donde la snake se va a mover (¿que caracter?)
 *
 * ... en la estructura del snake: actualizar el row y col de la cabeza
 *
 * Nota: esta funcion ignora la comida, paredes, y cuerpos de otras snakes
 * cuando se mueve la cabeza.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  snake_t* snake = &state->snakes[snum];
  char head_char = get_board_at(state, snake->head_row, snake->head_col);

  // Reemplazar la cabeza anterior por parte del cuerpo
  set_board_at(state, snake->head_row, snake->head_col, head_to_body(head_char));

  // Calcular nueva posición
  unsigned int new_row = get_next_row(snake->head_row, head_char);
  unsigned int new_col = get_next_col(snake->head_col, head_char);

  // Poner nueva cabeza
  set_board_at(state, new_row, new_col, head_char);

  // Actualizar coordenadas en la estructura
  snake->head_row = new_row;
  snake->head_col = new_col;
}



/**
 * Tarea 4.4
 *
 * Funcion de ayuda para update_state. Actualiza la cola de la snake...
 *
 * ... en el tablero: colocar un caracter blanco (spacio) donde se encuentra
 * la cola actualmente, y cambiar la nueva cola de un caracter de cuerpo (^<v>)
 * a un caracter de cola (wasd)
 *
 * ...en la estructura snake: actualizar el row y col de la cola
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  snake_t* snake = &state->snakes[snum];
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;

  char tail_char = get_board_at(state, row, col);

  // Borrar la cola actual
  set_board_at(state, row, col, ' ');

  // Calcular nueva posición de la cola
  unsigned int next_row = get_next_row(row, tail_char);
  unsigned int next_col = get_next_col(col, tail_char);

  // Obtener el carácter actual en esa posición
  char next_char = get_board_at(state, next_row, next_col);

  // Convertirlo a cola
  set_board_at(state, next_row, next_col, body_to_tail(next_char));

  // Actualizar posición en la estructura
  snake->tail_row = next_row;
  snake->tail_col = next_col;
}


/* Tarea 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    if (!state->snakes[i].live) continue;

    char next = next_square(state, i);

    if (next == '#' || is_snake(next)) {
      set_board_at(state, state->snakes[i].head_row, state->snakes[i].head_col, 'x');
      state->snakes[i].live = false;
    } else if (next == '*') {
      update_head(state, i);
      add_food(state);
    } else {
      update_head(state, i);
      update_tail(state, i);
    }
  }
}


/* Tarea 5 */
game_state_t* load_board(char* filename) {
  FILE* f = fopen(filename, "r");
  if (f == NULL) return NULL;

  game_state_t* state = malloc(sizeof(game_state_t));
  if (state == NULL) return NULL;

  // Paso 1: contar líneas
  state->num_rows = 0;
  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), f)) {
    state->num_rows++;
  }

  // Paso 2: reservar memoria para board
  state->board = malloc(sizeof(char*) * state->num_rows);
  if (state->board == NULL) return NULL;

  // Paso 3: volver a leer archivo desde el inicio
  rewind(f);

  for (unsigned int i = 0; i < state->num_rows; i++) {
    fgets(buffer, sizeof(buffer), f);
    size_t len = strlen(buffer);

    // Eliminar el salto de línea
    if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';

    state->board[i] = malloc(strlen(buffer) + 1);
    strcpy(state->board[i], buffer);
  }

  fclose(f);

  // Paso 4: dejar snakes vacías por ahora
  state->num_snakes = 0;
  state->snakes = NULL;

  return state;
}



/**
 * Tarea 6.1
 *
 * Funcion de ayuda para initialize_snakes.
 * Dada una structura de snake con los datos de cola row y col ya colocados,
 * atravezar el tablero para encontrar el row y col de la cabeza de la snake,
 * y colocar esta informacion en la estructura de la snake correspondiente
 * dada por la variable (snum)
*/
static void find_head(game_state_t* state, unsigned int snum) {
  snake_t* snake = &state->snakes[snum];
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;

  while (true) {
    char c = get_board_at(state, row, col);

    if (is_head(c)) {
      snake->head_row = row;
      snake->head_col = col;
      return;
    }

    unsigned int next_row = get_next_row(row, c);
    unsigned int next_col = get_next_col(col, c);

    // Solo nos movemos si no llegamos a la cabeza
    row = next_row;
    col = next_col;
  }
}






/* Tarea 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  unsigned int count = 0;

  // 1. Contar cuántas colas hay (serpientes)
  for (unsigned int row = 0; row < state->num_rows; row++) {
    for (unsigned int col = 0; state->board[row][col] != '\0'; col++) {
      if (is_tail(state->board[row][col])) {
        count++;
      }
    }
  }

  // 2. Crear arreglo de serpientes
  state->num_snakes = count;
  state->snakes = malloc(sizeof(snake_t) * count);

  // 3. Segunda pasada para llenar cada snake
  unsigned int index = 0;
  for (unsigned int row = 0; row < state->num_rows; row++) {
    for (unsigned int col = 0; state->board[row][col] != '\0'; col++) {
      if (is_tail(state->board[row][col])) {
        state->snakes[index].tail_row = row;
        state->snakes[index].tail_col = col;
        find_head(state, index);
        state->snakes[index].live = true;
        index++;
      }
    }
  }

  return state;
}

