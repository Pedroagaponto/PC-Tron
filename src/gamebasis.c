#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include "gamebasis.h"

struct game_basis basis;

sem_t can_refresh;

pthread_mutex_t mutex_sts;

int term_row, term_col;

int init_gamebasis()
{
	basis.status = 0;
	getmaxyx(stdscr, term_row, term_col);
	basis.size_row = term_row - 2;
	basis.size_col = term_col - 2;
	if (basis.size_row*2 < basis.size_col)
		basis.size_col = basis.size_row*2;
	else
		basis.size_row = basis.size_col/2;

	if (basis.size_row < 25)
		basis.status = -25;
	else
		if (alloc_mats() == -1)
			return -1;

	return 0;
}

int alloc_mats()
{
	int i;

	basis.field = (int**) calloc(basis.size_row, sizeof(int*));
	basis.l_field = (pthread_mutex_t**)
	calloc(basis.size_row, sizeof(pthread_mutex_t*));

	if ((basis.field == NULL) || (basis.l_field == NULL))
		return -1;

	for (i = 0; i < basis.size_row; i++)
	{
		basis.field[i] = (int*)
		calloc(basis.size_col, sizeof(int));
		basis.l_field[i] = (pthread_mutex_t*)
		calloc(basis.size_col, sizeof(pthread_mutex_t));
		if ((basis.field[i] == NULL) || (basis.l_field[i] == NULL))
			return -1;
	}
	if (init_matmutex() == -1)
	{
		free_mats();
		return -1;
	}
	return 0;
}

int init_matmutex()
{
	for (int i = 0; i < basis.size_row; i++)
		for (int j = 0; j < basis.size_col; j++)
			if (pthread_mutex_init(&basis.l_field[i][j], NULL) != 0)
				return -1;
	return 0;
}

void set_mat()
{
	for (int i = 0; i < basis.size_row; i++)
		for (int j = 0; j < basis.size_col; j++)
			basis.field[i][j] = 0;
}
void free_mats()
{
	int i;

	for (i = 0; i < basis.size_row; i++)
	{
		free(basis.field[i]);
		free(basis.l_field[i]);
	}
	free(basis.field);
	free(basis.l_field);
}
