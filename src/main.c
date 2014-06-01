#include <ncurses.h>

WINDOW *create_newwin(int win_y, int win_x, int starty, int startx);

int main(void)
{	
	WINDOW *new_win = NULL;
	int row, col, win_x, win_y;

	initscr();
	cbreak();	
	start_color();			

	getmaxyx(stdscr,row,col);
	win_y = (row > 40)? 40 : (row - 5);
	win_x = (col > 80)? 80 : (col - 10);
	if (win_y*2 < win_x)
		win_x = win_y*2;
	else
		win_y = win_x/2;
	
	refresh();
	new_win = create_newwin(win_y, win_x, (row-win_y)/2, (col-win_x)/2);
	getch();
	endwin();		
	
	return 0;
}

WINDOW *create_newwin(int win_y, int win_x, int starty, int startx)
{
	WINDOW *local_win;

	local_win = newwin(win_y, win_x, starty, startx);
	box(local_win, 0 , 0);		 
	wrefresh(local_win);

	return local_win;
}
