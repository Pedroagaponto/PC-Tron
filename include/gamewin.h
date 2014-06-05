void init_win();

WINDOW *create_win(int win_y, int win_x, int starty, int startx);

void destroy_win(WINDOW **local_win);

WINDOW* warn_win(int term_y, int term_x);

void printgame_win(WINDOW *local_win, int **mat, int row, int col);

