struct mem_cache_head
{
   size_t size;
   int num;
   struct mem_cache *next, *bock_next;
};

typedef struct mem_cache 
{
   struct mem_cache *prev, *next;
};

struct mem_pool_large
{
   void *start, *end;
   struct mem_pool_large *next;
};

struct mem_pool_head
{
   void *start, *end;
   struct mem_pool_head *next;
   struct mem_pool_large *extra, *tail;
};

struct mem_buffer
{
   void *start, *end;
   struct mem_buffer *prev, *next;
};

static struct mem_pool_head **pool_lst;

# define POOL_OFFSET(h, f) h+=(f+proxy_cnt+2)
# define POOL_SIZ PAGE_SIZ-sizeof(struct mem_pool_head)
