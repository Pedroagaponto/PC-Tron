#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include "gamebasis.h"
#include "gamewin.h"
#include "gamelogic.h"

sig_atomic_t directions[N_PLAYERS] = {RIGHT, LEFT};

sem_t can_we_play[N_PLAYERS];

sem_t can_continue;

pthread_t threads[N_PLAYERS+2];

struct key_map
{
	int player;
	int key;
	sig_atomic_t direction;
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
	int id = (int) num, move, old_move = 0;
	int row = basis.heads[id][0], col = basis.heads[id][1];

	while (1)
	{
		sem_wait(&can_we_play[id]);
		if ((basis.status == STATUS_PAUSE) ||
		   (basis.status == STATUS_END_MATCH))
		{
			sem_post(&can_continue);
			continue;
		}
		move = directions[id];
		move = (move == -old_move) ? old_move : move;
		old_move = move;
		switch (move)
		{
			case UP:
				row--;
				break;
			case DOWN:
				row++;
				break;
			case RIGHT:
				col++;
				break;
			case LEFT:
				col--;
				break;
		}
		if ((row <= 0) || (col <= 0)
		   || (row >= basis.size_row-1) || (col >= basis.size_col-1))
		{
			pthread_mutex_lock(&mutex_sts);
			basis.status = STATUS_GAME_OVER;
			basis.losers = (!basis.losers)? id+1 : DRAW;
			pthread_mutex_unlock(&mutex_sts);
		}
		else
		{
			pthread_mutex_lock(&basis.l_field[row][col]);
			if (basis.field[row][col] == 0)
				basis.field[row][col] = id+1;
			else
			{
				pthread_mutex_lock(&mutex_sts);
				basis.status = STATUS_GAME_OVER;
				basis.losers = (!basis.losers)? id+1 : DRAW;
				pthread_mutex_unlock(&mutex_sts);
			}
			pthread_mutex_unlock(&basis.l_field[row][col]);

			pthread_mutex_lock(&basis.l_heads);
			basis.heads[id][0] = row;
			basis.heads[id][1] = col;
			pthread_mutex_unlock(&basis.l_heads);
		}
		sem_post(&can_continue);
	}
	return NULL;
}

void* read_key(void *arg)
{
	int i, c, paused = 0;

	while (1)
	{
		c = getch();
		if ((paused > 0) && (c != PAUSED) && (c != KEY_F(1)))
			continue;
		switch(c)
		{
			case (KEY_F(1)):
				pthread_mutex_lock(&mutex_sts);
				basis.status = STATUS_EXIT;
				pthread_mutex_unlock(&mutex_sts);
				break;
			case (KEY_RESIZE):
				pthread_mutex_lock(&mutex_sts);
				basis.status = STATUS_RESIZE;
				pthread_mutex_unlock(&mutex_sts);
				break;
			case (PAUSED):
				paused++;
				if (paused == 1)
				{
					pthread_mutex_lock(&mutex_sts);
					basis.status = STATUS_PAUSE;
					pthread_mutex_unlock(&mutex_sts);
				}
				else
				{
					paused = 0;
					pthread_mutex_lock(&mutex_sts);
					basis.status = STATUS_NORMAL;
					pthread_mutex_unlock(&mutex_sts);
				}

			default:
				for (i = 0; mapping[i].player; i++)
					if (c == mapping[i].key)
					{
						directions[mapping[i].player-1]=
						mapping[i].direction;
					}
				break;
		}
	}

	return arg;
}

int diff (struct timespec start, struct timespec end)
{
	struct timespec temp;
	int ret;

	if ((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1e9 + end.tv_nsec - start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}

	ret = spec_to_usec(temp);

	return (ret > REFRESH_US) ? REFRESH_US : ret;
}

int spec_to_usec(struct timespec time)
{
	int temp;

	/* Nano to micro */
	temp = time.tv_nsec / 1e3;
	temp += time.tv_sec * 1e6;

	return temp;
}

void check_draw()
{
	int i, j;

	for (i = 0; i < N_PLAYERS; i++)
		for (j = i + 1; j < N_PLAYERS; j++)
			if ((i != j) &&
			   (basis.heads[i][0] == basis.heads[j][0]) &&
			   (basis.heads[i][1] == basis.heads[j][1]))
			{
				pthread_mutex_lock(&mutex_sts);
				basis.status = STATUS_GAME_OVER;
				basis.losers = DRAW;
				pthread_mutex_unlock(&mutex_sts);
			}
}

void initvar_pthread()
{
	for (int i = 0; i < N_PLAYERS; i++)
		sem_init(&can_we_play[i], 0, 0);
	sem_init(&can_refresh, 0, 0);
	sem_init(&can_continue, 0, 0);
	sem_init(&screen_ready, 0, 0);
	pthread_mutex_init(&mutex_sts, NULL);
	pthread_mutex_init(&basis.l_heads, NULL);
}

void create_threads()
{
	int i;

	if (pthread_create(&threads[2], NULL, &refresh_game, NULL))
	{
		fprintf(stderr, "Cannot create thread refresh_game\n");
		exit(-1);
	}
	if (pthread_create(&threads[3], NULL, &read_key, NULL))
	{
		fprintf(stderr, "Cannot create thread read_key\n");
		exit(-1);
	}
	for (i = 0; i < N_PLAYERS; i++)
	{
		if (pthread_create(&threads[i], NULL, &worm, (void *) i))
		{
			fprintf(stderr, "Cannot create thread worm %d\n", i);
			exit(-1);
		}
	}
}

void* judge(void *arg)
{
	int i;

	initvar_pthread();
	create_threads();

	while(1)
	{
		sem_wait(&screen_ready);
		for (i = 0; i < N_PLAYERS; i++)
			sem_post(&can_we_play[i]);

		for (i = 0; i < N_PLAYERS; i++)
			sem_wait(&can_continue);

		check_draw();
		sem_post(&can_refresh);
		usleep(REFRESH_US);
		sem_post(&can_refresh);
	}
	return arg;
}
