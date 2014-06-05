#include <ncurses.h>
#include "gamewin.h"

/* The stdscr window is initiated to allow change of colors and the use
of especial keys. Also is initiated the disabing of echoes and the invisible cursor */
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

	local_win = newwin(win_y, win_x, starty, startx);
	box(local_win, 0 , 0);
	wrefresh(local_win);

	return local_win;
}

void destroy_win(WINDOW **local_win)
{
	werase(*local_win);
	wrefresh(*local_win);
	delwin(*local_win);
	*local_win = NULL;
}

/* If the terminal's dimension is big enough, creates a window that shows a
complete war_win message. Else, sends a short message asking to resize
(maximize) the terminal's dimension */
WINDOW* warn_win(int term_y, int term_x)
{
	WINDOW *adv = NULL;

	adv = create_win(7, 34, (term_y-7)/2, (term_x-34)/2);
	if (adv != NULL)
	{
		mvwprintw(adv, 1, 2, "The game's field doesn't fit in");
		mvwprintw(adv, 2, 1, "the actual terminal's dimension.");
		mvwprintw(adv, 4, 8, "Resize+ the terminal");
		mvwprintw(adv, 5, 9, "Or quit with <F1>");
		wrefresh(adv);
	}
	else
	{
		mvprintw(0, 0, "RESIZE+");
		refresh();
	}

	return adv;
}

void printgame_win(WINDOW *local_win, int **mat, int row, int col)
{
	int i, j;

	start_color();
	init_pair(1, COLOR_WHITE, COLOR_CYAN);
	init_pair(2, COLOR_WHITE, COLOR_MAGENTA);

	for (i = 1; i <= row; i++)
		for (j = 1; j <= col; j++)
			if (mat[i][j] != 0)
			{
				if ((mat[i][j]*mat[i][j]) == 1)
					attron(COLOR_PAIR(1));
				else
					attron(COLOR_PAIR(2));
				if (mat[i][j] > 0)
					mvwprintw(local_win, i, j, "o");
				else
					mvwprintw(local_win, i, j,"O");
			}
	wrefresh(local_win);
	attron(COLOR_PAIR(0));
}
