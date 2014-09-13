# define HTTP_GET 11
# define HTTP_HEAD 12
# define HTTP_POST 13

void lida_breakline (char *);
int lida_hashkey (char *);
char *hash_conflictsearch (int, char *);

struct http_request_t
{
   char *method;
   char *url;
   char *protocal;
   struct http_attrihash *list[29];
   size_t offset;
};

struct http_attrihash 
{
   char *content;
   void *attri;
   struct http_attrihash *next;
};

static unsigned parse_flag = 1;
static unsigned parse_start = 1;

char *method_lst[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT", "OPTIONS"};
# define METHOD_N 8;
