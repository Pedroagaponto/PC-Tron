#include <pthread.h>
#include <semaphore.h>

#define UP 'w'
#define DOWN 's'
#define RIGHT 'd'
#define LEFT 'a'
#define N_PLAYERS 2
#define REFRESH_US 400000
#define STATUS_RESIZE -10
#define STATUS_ADV -11
#define STATUS_SHORT_MSG -12
#define STATUS_NORMAL 0
#define STATUS_DRAW -3
#define STATUS_PLAYER_LOSE(x) -x
#define STATUS_EXIT -20
#define STATUS_TOO_SMALL -25

struct game_basis
{
	int status;
	int **field;
	pthread_mutex_t **l_field;
	int size_row, size_col;
	pthread_mutex_t l_heads;
	int heads[N_PLAYERS][2];
};

extern struct game_basis basis;

extern int term_row, term_col;

extern sem_t can_refresh;

extern pthread_mutex_t mutex_sts;

int init_gamebasis();

int alloc_mats();

void free_mats();

int init_matmutex();

void set_mat();

