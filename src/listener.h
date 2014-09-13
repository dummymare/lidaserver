# include <pthread.h>

struct event_data
{
   int fd;
   int pfd;
   struct proxy_node *node; 
};

void listener_proc (int, int, int);
int lida_timer_init (void);
void lida_timer_add (int);
struct flock *lida_lock_init (int);

static int proxy_cnt = 0; 

static unsigned full_flag = 0;

struct timer_head event;

struct mem_cache_head *listener_mem_cache;

static int epfd;

pthread_mutex_t pth_lock;

# define IF_PROXY(A) (A)&0x0000FFFF  

# define SET_NONBLOCK(A) fcntl(A, F_SETFL, fcntl (A, F_GETFL, 0) | O_NONBLOCK)   

char err_page[] = "HTTP/1.1 404 NOTFOUND\r\n
                  Server: LIDA/1.0\r\n
		  Content-Type: text/html\r\n
		  Content-Length: 112\r\n
		  \r\n
		  <html><title>server-message</title>
		  <body><p style=\"font-size:150%\">404 NOT FOUND</p>
		  </body></html>";
