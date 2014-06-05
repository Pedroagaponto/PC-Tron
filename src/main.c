#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "gamewin.h"

int main(void)
{
	WINDOW *game_field = NULL, *adv = NULL;
	int x, y, row, col, win_x, win_y, c;

	init_win();

	/* Creates a square game field adapted to the terminal's dimension */
	getmaxyx(stdscr,row,col);
	win_y = row - 2;
	win_x = col -2;
	if (win_y*2 < win_x)
		win_x = win_y*2;
	else
		win_y = win_x/2;
	y = (row-win_y)/2;
	x = (col-win_x)/2;
	mvprintw(y+win_y, (col-19)/2, "Press F1 to exit");
	refresh();

	game_field = create_win(win_y, win_x, y, x);
	/* Identifies possible changes on the terminal's dimension, adapting the
	game field when is possible or sending warnings when it doesn't */
	while ((c = getch()) != KEY_F(1))
	{
		if (c == KEY_RESIZE)
		{
			erase();
			refresh();
			if (adv != NULL)
				destroy_win(&adv);
			if(game_field != NULL)
				destroy_win(&game_field);
			getmaxyx(stdscr,row,col);
			y = (row-win_y)/2;
			x = (col-win_x)/2;
			if ((row >= win_y+2) && (col >= win_x+2))
			{
				game_field = create_win(win_y, win_x, y, x);
				mvprintw(y+win_y,(col-19)/2,"Press F1 to exit");
			}
			else
				adv = warn_win(row, col);
			refresh();
		}
	}

	endwin();
	return 0;
}

