#include <stdlib.h>
#include <ncurses.h>
#include "gamebasis.h"
#include "gamewin.h"
#include "gamelogic.h"

WINDOW *game_field, *adv;

/* The stdscr window is initiated to allow change of colors and the use
of especial keys. Also is initiated the disabing of echoes and the
invisible cursor */
void init_win()
{
	initscr();
	cbreak();
	start_color();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
}

WINDOW *create_win(int win_y, int win_x, int starty, int startx)
{
	WINDOW *local_win;

	refresh();
	local_win = newwin(win_y, win_x, starty, startx);
	box(local_win, 0 , 0);
	wrefresh(local_win);
	refresh();

	return local_win;
}

void destroy_win(WINDOW **local_win)
{
	werase(*local_win);
	wrefresh(*local_win);
	delwin(*local_win);
	*local_win = NULL;
}

/* If the terminal's dimension is big enough, creates a window that shows
a complete war_win message. Else, sends a short message asking to resize
(maximize) the terminal's dimension */
void createwarn_win()
{
	adv = create_win(7, 34, (term_row-7)/2, (term_col-34)/2);

	if (adv != NULL)
	{
		mvwprintw(adv, 1, 2, "The game's field doesn't fit in");
		mvwprintw(adv, 2, 1, "the actual terminal's dimension.");
		mvwprintw(adv, 4, 8, "Resize+ the terminal");
		mvwprintw(adv, 5, 9, "Or quit with <F1>");
		wrefresh(adv);
		pthread_mutex_lock(&mutex_sts);
		basis.status = STATUS_ADV;
		pthread_mutex_unlock(&mutex_sts);
	}
	else
	{
		mvprintw(0, 0, "RESIZE+");
		refresh();
		pthread_mutex_lock(&mutex_sts);
		basis.status = STATUS_SHORT_MSG;
		pthread_mutex_unlock(&mutex_sts);
	}
}

void createfield_win()
{
	int y = (term_row-basis.size_row)/2;
	int x = (term_col-basis.size_col)/2;

	mvprintw(y+basis.size_row, (term_col-19)/2, "Press F1 to exit");
	refresh();

	game_field = create_win(basis.size_row, basis.size_col, y, x);
}

void destroy_wins()
{
	if (adv != NULL)
		destroy_win(&adv);
	if(game_field != NULL)
		destroy_win(&game_field);
}

void* refresh_game(void *arg)
{
	game_field = NULL;
	adv = NULL;

	init_win();
	createfield_win();

	/* Identifies possible changes on the terminal's dimension,
adapting the game field when is possible or sending warnings when it
doesn't */
	while (1)
	{
		//without synchronization (TODO)
		//sem_wait(&can_refresh);
		pthread_mutex_lock(&mutex_sts);
		if (basis.status == STATUS_RESIZE)
		{
			pthread_mutex_unlock(&mutex_sts);
			erase();
			destroy_wins();
			refresh();
			getmaxyx(stdscr, term_row, term_col);
			if ((term_row >= basis.size_row+2) &&
			   (term_col >= basis.size_col+2))
			{
				basis.status = 0;
				createfield_win();
			}
			else
				createwarn_win();
			refresh();
		}
		else if (basis.status == STATUS_EXIT)
		{
			pthread_mutex_unlock(&mutex_sts);
			destroy_wins();
			free_mats();
			endwin();
			exit(1);
		}
		else
			pthread_mutex_unlock(&mutex_sts);
		refresh();
	}

	return arg;
}
