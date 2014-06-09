#include <pthread.h>
#include <semaphore.h>

#define UP 1
#define DOWN -1
#define RIGHT 2
#define LEFT -2
#define N_PLAYERS 2
#define PAUSED 'p'
#define REFRESH_US 100000
#define STATUS_RESIZE -10
#define STATUS_END_MATCH -40
#define STATUS_ADV -11
#define STATUS_PAUSE 30
#define STATUS_SHORT_MSG -12
#define STATUS_NORMAL 0
#define STATUS_GAME_OVER -1
#define STATUS_EXIT -20
#define STATUS_TOO_SMALL -25
#define DRAW -3

struct game_basis
{
	int status;
	int losers;
	int **field;
	pthread_mutex_t **l_field;
	int size_row, size_col;
	pthread_mutex_t l_heads;
	int heads[N_PLAYERS][2];
};

extern struct game_basis basis;

extern int term_row, term_col;

extern sem_t can_refresh;

extern sem_t screen_ready;

extern pthread_mutex_t mutex_sts;

int init_gamebasis();

int alloc_mats();

void free_mats();

int init_matmutex();

void set_mat();

