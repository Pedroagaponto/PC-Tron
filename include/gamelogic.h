void* worm(void *num);

void* read_key(void *arg);

int spec_to_usec(struct timespec time);

int diff(struct timespec old, struct timespec act);

void check_draw();

void initvar_pthread();

void create_threads();

void* judge(void *arg);
