# include <dlfcn.h>
# include <libxml/parser.h>
# include <libxml/tree.h>
# include <libxml/xpath.h>

jmp_buf env = 0;

void      port_to_ghost (void);
xmlDocPtr lida_docheck      (char *, char *);
void lida_readhosts (void);

struct proxy_pass
{
   char *match;
   unsigned cache_flag;
   struct proxy_group *proxy;
};

struct config_info
{
   char *hostname;
   char *path;
   char *frontpage;
   struct proxy_pass *active;
};

struct config_info *httphost;

struct cache_options
{
   unsigned disk_flag;
   size_t mem_max;
   size_t disk_max;
   char *match;
};

struct server_basic
{
   int port;
   int listener;
   int connect;
   time_t keep_alive;
   struct cache_options *cache;
};

static struct server_basic true_type;

union proxy_status
{
   void *addr;
   unsigned long s_stat;
};

struct proxy_node
{
   char *ip;
   unsigned short port;
   unsigned int weight;
   unsigned b_type;
   union proxy_status status;
   int *fd;
   int act_cnt;
   float key;
   struct proxy_node *next, *prev;
};

struct proxy_group
{
   char *id;
   struct proxy_node *node;
};

static struct proxy_group *proxy_lst;
