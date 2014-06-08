#include <pthread.h>

#define UP 'w'
#define DOWN 's'
#define RIGHT 'd'
#define LEFT 'a'
#define N_PLAYERS 2

struct game_basis
{
	int status;
	int **field;
	pthread_mutex_t **l_field;
	int size_row, size_col;
	int heads[N_PLAYERS][2];
};

extern struct game_basis basis;

extern int term_row, term_col;

int init_gamebasis();

int alloc_mats();

void free_mats();

int init_matmutex();
