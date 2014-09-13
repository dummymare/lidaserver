# define READ_EVENT 0
# define PROXY_EVENT 1
# define SOCKET_EVENT 2

struct thread_data_t
{
   pthread_t id;
   int fd;
   int pfd;

   unsigned tmp_flag : 4;
   unsigned type : 4;

   void *data;
   FILE *fp;

   struct mem_cache_head *pool;
};

struct thread_pool_h
{
   struct queue_chain_ctl *h;
   struct mem_cache_head *pool;
   struct mem_cache_head *spool;
};
