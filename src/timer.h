# include "thread.h"

struct timer_h
{
   int max;
   int cnt;

   struct rbtree_t *s;
   struct mem_cache_head *pool;
   struct mem_cache_head *rbt_pool;
};

struct timer_t
{
   int fd;
   int pfd;

   time_t time;
   pthread_t thread_id;
};

# define HUGE_TIMER 
