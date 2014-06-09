#include <stdlib.h>
#include <ncurses.h>
#include "gamebasis.h"
#include "gamewin.h"
#include "gamelogic.h"

WINDOW *game_field, *adv;
int old_heads[N_PLAYERS][2];

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

void createwin_win(int winner)
{
	adv = create_win(4, 32, (term_row-4)/2, (term_col-26)/2);

	if (adv != NULL)
	{
		if (winner == DRAW)
			mvwprintw(adv, 1, 14, "DRAW!");
		else
			mvwprintw(adv, 1, 11, "WORM %d WINS!", (winner%2)+1);
		mvwprintw(adv, 2, 6, "Press any key to exit");
		wrefresh(adv);
	}
	pthread_mutex_lock(&mutex_sts);
	basis.status = STATUS_END_MATCH;
	pthread_mutex_unlock(&mutex_sts);
}

void createpause_win()
{
	adv = create_win(4, 32, (term_row-4)/2, (term_col-32)/2);

	if (adv != NULL)
	{
		mvwprintw(adv, 1, 9, "GAME IS PAUSED!");
		mvwprintw(adv, 2, 1, "Press p <unpause> or F1 <quit>");
		wrefresh(adv);
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

void refresh_resize()
{
	erase();
	destroy_wins();
	refresh();
	getmaxyx(stdscr, term_row, term_col);
	if ((term_row >= basis.size_row+2) &&
	   (term_col >= basis.size_col+2))
	{
		pthread_mutex_lock(&mutex_sts);
		basis.status = 0;
		pthread_mutex_lock(&mutex_sts);
		createfield_win();
	}
	else
		createwarn_win();
	refresh();
}


void refresh_exit()
{
	destroy_wins();
	free_mats();
	endwin();
	exit(1);

}

void refresh_move()
{
	pthread_mutex_lock(&basis.l_heads);
	for (int i = 0; i < N_PLAYERS; i++)
	{
		wattron(game_field, COLOR_PAIR(i+1));
		mvwprintw(game_field, old_heads[i][0], old_heads[i][1], "o");
		mvwprintw(game_field, basis.heads[i][0], basis.heads[i][1],"O");
	}
	wattron(game_field, COLOR_PAIR(0));
	refresh_oldheads();
	pthread_mutex_unlock(&basis.l_heads);
	wrefresh(game_field);
}

void refresh_allfield()
{
	werase(game_field);
	for (int i = 0; i < basis.size_row; i++)
		for (int j = 0; j < basis.size_col; j++)
		{
			pthread_mutex_lock(&basis.l_field[i][j]);
			if (basis.field[i][j] != 0)
				mvwprintw(game_field, i, j, "o"); 
			pthread_mutex_unlock(&basis.l_field[i][j]);

		}
	pthread_mutex_lock(&basis.l_heads);
	mvwprintw(game_field, basis.heads[0][0], basis.heads[0][1],"O");
	mvwprintw(game_field, basis.heads[1][0], basis.heads[1][1],"O");
	pthread_mutex_unlock(&basis.l_heads);
	box(game_field, 0 , 0);
	wrefresh(game_field);
}

void refresh_oldheads()
{
	for (int i = 0; i < N_PLAYERS; i++)
	{
		old_heads[i][0] = basis.heads[i][0];
		old_heads[i][1] = basis.heads[i][1];
	}
}

void* refresh_game(void *arg)
{
	game_field = NULL;
	adv = NULL;

	createfield_win();
	refresh_oldheads();
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);

	/* Identifies possible changes on the terminal's dimension,
adapting the game field when is possible or sending warnings when it
doesn't */
	while (1)
	{
		sem_post(&screen_ready);
		sem_wait(&can_refresh);
		pthread_mutex_lock(&mutex_sts);
		switch (basis.status)
		{
			case (STATUS_NORMAL):
				pthread_mutex_unlock(&mutex_sts);
				refresh_move();
				break;
			case (STATUS_RESIZE):
				pthread_mutex_unlock(&mutex_sts);
				refresh_resize();
				break;
			case (STATUS_PAUSE):
				if (adv != NULL)
					destroy_win(&adv);
				pthread_mutex_unlock(&mutex_sts);
				createpause_win();
				break;
			case (STATUS_EXIT):
				pthread_mutex_unlock(&mutex_sts);
				refresh_exit();
			case (STATUS_GAME_OVER):
				pthread_mutex_unlock(&mutex_sts);
				createwin_win(basis.losers);
				getch();
				refresh_exit();
			default:
				pthread_mutex_unlock(&mutex_sts);
		}
		refresh();
	}

	return arg;
}

