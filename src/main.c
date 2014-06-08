#include <ncurses.h>
#include <pthread.h>
#include "gamebasis.h"
#include "gamewin.h"
#include "gamelogic.h"

int main(void)
{
	pthread_t thread;

	init_win();

	if (init_gamebasis() == -1)
	{
		printw("Error during the allocation memory process.\n");
		refresh();
		endwin();

		return 0;
	}

	if (pthread_create(&thread, NULL, &judge, NULL) != 0)
		printw("Error during the creation thread process.\n");
	else
		pthread_join(thread, NULL);

	free_mats();
	endwin();

	return 0;
}

