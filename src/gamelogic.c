#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include "gamebasis.h"
#include "gamewin.h"
#include "gamelogic.h"

char directions[N_PLAYERS] = {RIGHT, LEFT};

sem_t can_we_play[N_PLAYERS];

sem_t can_continue;

pthread_t threads[N_PLAYERS+2];

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
	int row = basis.heads[id][0], col = basis.heads[id][1];

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
		if ((row <= 0) || (col <= 0))
		{
			pthread_mutex_lock(&mutex_sts);
			basis.status = STATUS_PLAYER_LOSE(id);
			pthread_mutex_unlock(&mutex_sts);
		}
		else if ((row >= basis.size_row-1) || (col >= basis.size_col-1))
		{
			pthread_mutex_lock(&mutex_sts);
			basis.status = STATUS_PLAYER_LOSE(id);
			pthread_mutex_unlock(&mutex_sts);
		}
		else
		{
			pthread_mutex_lock(&basis.l_field[row][col]);
			if (basis.field[row][col] == 0)
				basis.field[row][col] = id;
			else
			{
				pthread_mutex_lock(&mutex_sts);
				basis.status = STATUS_PLAYER_LOSE(id);
				pthread_mutex_unlock(&mutex_sts);
			}
			pthread_mutex_unlock(&basis.l_field[row][col]);

			basis.heads[id][0] = row;
			basis.heads[id][1] = col;
		}
		sem_post(&can_continue);
	}
	return NULL;
}

void* read_key(void *arg)
{
	int i, c;

	while (1)
	{
		refresh();
		c = getch();
		if (c == KEY_F(1))
		{
			pthread_mutex_lock(&mutex_sts);
			basis.status = STATUS_EXIT;
			pthread_mutex_unlock(&mutex_sts);
		}
		else if (c == KEY_RESIZE)
		{
			pthread_mutex_lock(&mutex_sts);
			basis.status = STATUS_RESIZE;
			pthread_mutex_unlock(&mutex_sts);
		}
		else
			for (i = 0; mapping[i].player; i++)
				if (c == mapping[i].key)
				{
					directions[mapping[i].player - 1] =
					mapping[i].direction;
					break;
				}
		refresh();
	}

	return arg;
}

int diff (struct timespec start, struct timespec end)
{
	struct timespec temp;
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

	return spec_to_usec(temp);
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
		for (j = 0; j < N_PLAYERS; j++)
			if ((i != j) &&
			   (basis.heads[i][0] == basis.heads[j][0]) &&
			   (basis.heads[i][1] == basis.heads[j][0]))
			{
				pthread_mutex_lock(&mutex_sts);
				basis.status = STATUS_DRAW;
				pthread_mutex_unlock(&mutex_sts);
			}
}

void initvar_pthread()
{
	for (int i = 0; i < N_PLAYERS; i++)
		sem_init(&can_we_play[i], 1, 0);
	sem_init(&can_refresh, 1, 0);
	sem_init(&can_continue, 1, 0);
	pthread_mutex_init(&mutex_sts, NULL);
}

void create_threads()
{
	int i;

	for (i = 0; i < N_PLAYERS; i++)
	{
		if (pthread_create(&threads[i], NULL, &worm, (void *) i))
		{
			fprintf(stderr, "Cannot create thread worm %d\n", i);
			exit(-1);
		}
	}
	if (pthread_create(&threads[i], NULL, &read_key, NULL))
	{
		fprintf(stderr, "Cannot create thread read_key\n");
		exit(-1);
	}
	if (pthread_create(&threads[i+1], NULL, &refresh_game, NULL))
	{
		fprintf(stderr, "Cannot create thread refresh_game\n");
		exit(-1);
	}
}

void join_threads()
{
	for (int i = 0; i < N_PLAYERS+2; i++)
		pthread_join(threads[i], NULL);
}

void* judge(void *arg)
{
	int i;
	struct timespec old_time, act_time;

	clock_gettime(CLOCK_MONOTONIC, &old_time);
	initvar_pthread();
	create_threads();
	join_threads();

	while(1)
	{
		for (i = 0; i < N_PLAYERS; i++)
			sem_post(&can_we_play[i]);

		check_draw();
		for (i = 0; i < N_PLAYERS; i++)
			sem_wait(&can_continue);
		sem_post(&can_refresh);
		refresh();
		clock_gettime(CLOCK_MONOTONIC, &act_time);
		usleep(1000 - diff(old_time, act_time));
		old_time = act_time;
	}
	return arg;
}
