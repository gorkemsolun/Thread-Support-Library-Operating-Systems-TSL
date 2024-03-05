int tsl_init(int salg);

int tsl_create_thread(void (*tsf)(void *), void *targ);

int tsl_yield(int tid);

int tsl_exit();

int tsl_join(int tid);

int tsl_cancel(int tid);

int tsl_gettid();