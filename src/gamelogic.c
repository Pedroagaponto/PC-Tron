#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define UP 'w'
#define DOWN 's'
#define RIGHT 'd'
#define LEFT 'a'
#define N_PLAYERS 2
//#define TEST

struct game_logic
{
	int status;
	int **field;
	pthread_mutex_t **l_field;
	int size_row, size_col;
	int heads[N_PLAYERS][2];
};

struct game_logic field_logic;

char directions[N_PLAYERS] = {RIGHT, LEFT};

struct key_map
{
	int player;
	int key;
	char direction;
};

const struct key_map mapping[] = {
	{1, 'w', UP},
	{1, 's', DOWN},
	{1, 'd', RIGHT},
	{1, 'a', LEFT},
	{2, KEY_UP, UP},
	{2, KEY_DOWN, DOWN},
	{2, KEY_RIGHT, RIGHT},
	{2, KEY_LEFT, LEFT},
	{0, 0, 0}
};

void read_key()
{
	int i, c;

	while (1)
	{
		c = getch();
		if (c == KEY_F(1))
			field_logic.status = -1;
		for (i = 0; mapping[i].player; i++)
			if (c == mapping[i].key)
			{
				directions[mapping[i].player - 1] =
				mapping[i].direction;
				break;
			}
		mvprintw(0, 0, "%c %c", directions[0], directions[1]);
		refresh();
	}
}

/* TODO */
int diff(struct timespec old, struct timespec act)
{
	return old.tv_sec+act.tv_sec-old.tv_sec-act.tv_sec;
}

void judge()
{
	struct timespec old_time, act_time;

	clock_gettime(CLOCK_MONOTONIC, &old_time);

	while(1)
	{
		//checks mutex before execution (window's ready)

		//allows worms to play <3

		//waits the moves to be concluded

		//checks de rules

		//asks the interface to update

		clock_gettime(CLOCK_MONOTONIC, &act_time);
		usleep(1000 - diff(old_time, act_time));
		old_time = act_time;
	}
}

#ifdef TEST
int main()
{
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	read_key();

	endwin();

	return 0;
}
#endif /* TEST */
