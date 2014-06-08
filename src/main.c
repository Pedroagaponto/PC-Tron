#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "gamebasis.h"
#include "gamewin.h"

int main(void)
{
	WINDOW *game_field = NULL, *adv = NULL;
	int x, y, c;

	init_win();
	if (init_gamebasis() == -1)
	{
		fprintf(stderr, "ERROR during the allocation memory process\n");
		return -1;
	}
	/* Creates a square game field adapted to the terminal's dimension */
	y = (term_row-basis.size_row)/2;
	x = (term_col-basis.size_col)/2;
	mvprintw(y+basis.size_row, (term_col-19)/2, "Press F1 to exit");
	refresh();

	game_field = create_win(basis.size_row, basis.size_col, y, x);
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
			getmaxyx(stdscr,term_row,term_col);
			y = (term_row-basis.size_row)/2;
			x = (term_col-basis.size_col)/2;
			if ((term_row >= basis.size_row+2) &&
			   (term_col >= basis.size_col+2))
			{
				game_field =
				create_win(basis.size_row, basis.size_col,y, x);
				move(y+basis.size_row,(term_col-19)/2);
				printw("Press F1 to exit");
			}
			else
				adv = warn_win(term_row, term_col);
			refresh();
		}
	}

	endwin();
	return 0;
}

