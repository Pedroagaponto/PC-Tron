#include <ncurses.h>

WINDOW *create_win(int win_y, int win_x, int starty, int startx);
void destroy_win(WINDOW **local_win);
WINDOW* warning(int term_y, int term_x);

int main(void)
{
	WINDOW *game_field = NULL, *adv = NULL;
	int x, y, row, col, win_x, win_y, c;

	/* The stdscr window is initiated to allow change of colors and the use
	of especial keys. Also is initiated the disabing of echoes and the 
	invisible cursor */
	initscr();
	cbreak();	
	start_color();			
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	
	/* Creates a square game field adapted to the terminal's dimension */
	getmaxyx(stdscr,row,col);
	win_y = row - 2;
	win_x = col - 2;
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
				adv = warning(row, col);
			refresh();
		}
	}
	
	endwin();		
	return 0;
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
complete warning message. Else, sends a short message asking to resize 
(maximize) the terminal's dimension */
WINDOW* warning(int term_y, int term_x)
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
