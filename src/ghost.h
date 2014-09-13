# define LIDA_BUFSIZ   4096

void          backend            (void);
int           passiveTCP         (void);
void          lida_fork          (void);
struct flock *lida_lock_init     (void);
void          check_client       (int);
void          read_exit          (int);

static int msock;
static int *listener_lst, cache_pid;
static unsigned QUIT_flag = 0;
