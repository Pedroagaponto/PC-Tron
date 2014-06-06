#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define UP 'w'
#define DOWN 's'
#define RIGHT 'd'
#define LEFT 'a'
#define N_PLAYERS 2
//#define TEST

struct game_logic
{
	int status;
	int **field;
	pthread_mutex_t **l_field;
	int size_row, size_col;
	int heads[N_PLAYERS][2];
};

struct game_logic f_logic;

char directions[N_PLAYERS] = {RIGHT, LEFT};

sem_t can_we_play[N_PLAYERS];

pthread_barrier_t barrier;

struct key_map
{
	int player;
	int key;
	char direction;
};

const struct key_map mapping[] = {
	{1, 'w', UP},
	{1, 's', DOWN},
	{1, 'd', RIGHT},
	{1, 'a', LEFT},
	{2, KEY_UP, UP},
	{2, KEY_DOWN, DOWN},
	{2, KEY_RIGHT, RIGHT},
	{2, KEY_LEFT, LEFT},
	{0, 0, 0}
};

void* worm(void *num)
{
	int id = (int) num;
	int row = f_logic.heads[id][0], col = f_logic.heads[id][1];

	while (1)
	{
		sem_wait(&can_we_play[id]);
		switch (directions[id])
		{
			case UP:
				row++;
			case DOWN:
				row--;
			case RIGHT:
				col++;
			case LEFT:
				col--;
		}
		if ((row < 0) || (col < 0 ))
		{
			f_logic.status = -(id);
		} else {
			pthread_mutex_lock(&f_logic.l_field[row][col]);
			if (f_logic.field[row][col] == 0)
				f_logic.field[row][col] = id;
			else
				f_logic.status = -(id);
			pthread_mutex_unlock(&f_logic.l_field[row][col]);

			f_logic.heads[id][0] = row;
			f_logic.heads[id][1] = col;
		}
	}
}

void* read_key(void *param)
{
	int i, c = (int) param;

	while (1)
	{
		c = getch();
		if (c == KEY_F(1))
			f_logic.status = -1;
		for (i = 0; mapping[i].player; i++)
			if (c == mapping[i].key)
			{
				directions[mapping[i].player - 1] =
				mapping[i].direction;
				break;
			}
		mvprintw(0, 0, "%c %c", directions[0], directions[1]);
		refresh();
	}
}

/* TODO */
int diff(struct timespec old, struct timespec act)
{
	return old.tv_sec+act.tv_sec-old.tv_sec-act.tv_sec;
}

void check_draw()
{
	int i, j;

	for (i = 0; i < N_PLAYERS-1; i++)
		for (j = 1; j < N_PLAYERS; j++)
			if ((i != j) &&
			   (f_logic.heads[i][0] == f_logic.heads[j][0]) &&
			   (f_logic.heads[i][1] == f_logic.heads[j][0]))
				f_logic.status = -3;
}
void create_threads(pthread_t **threads)
{
	int i;

	*threads = (pthread_t*) calloc(N_PLAYERS+1, sizeof(pthread_t));
	pthread_barrier_init(&barrier, NULL, N_PLAYERS);
	for (i = 0; i < N_PLAYERS; i++)
		sem_init(&can_we_play[i], 1, 0);

	for (i = 0; i < N_PLAYERS; i++)
	{
		if (pthread_create(threads[i], NULL, worm, (void *) i))
		{
			fprintf(stderr, "Cannot create thread worm %d\n", i);
			exit(-1);
		}
	}
	if (pthread_create(threads[i], NULL, read_key, NULL))
	{
		fprintf(stderr, "Cannot create thread read_key\n");
		exit(-1);
	}
}

void judge()
{
	int i;
	struct timespec old_time, act_time;
	pthread_t *threads = NULL;

	clock_gettime(CLOCK_MONOTONIC, &old_time);
	create_threads(&threads);

	while(1)
	{
		//checks mutex before execution (window's ready)

		for (i = 0; i < N_PLAYERS; i++)
			sem_post(&can_we_play[i]);

		pthread_barrier_wait(&barrier);
		check_draw();

		//asks the interface to update

		clock_gettime(CLOCK_MONOTONIC, &act_time);
		usleep(1000 - diff(old_time, act_time));
		old_time = act_time;
	}
}

#ifdef TEST
int main()
{
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	read_key();

	endwin();

	return 0;
}
#endif /* TEST */
