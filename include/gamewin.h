void init_win();

WINDOW *create_win(int win_y, int win_x, int start_row, int start_col);

void destroy_win(WINDOW **local_win);

void destroy_wins();

void createwarn_win();

void createwin_win(int losers);

void createpause_win();

void createfield_win();

void refresh_resize();

void refresh_exit();

void refresh_allfield();

void refresh_move();

void refresh_oldheads();

void* refresh_game(void *arg);
