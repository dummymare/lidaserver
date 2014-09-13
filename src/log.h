# define ERROR -1
# define TRUE   1
# define FALSE  0

# define LIDA_ERR 44
# define LIDA_ACCESS 45

# define ERR_EXIT(A) {lidalog (LIDA_ERR, A); exit (1);}
# define ERR_RET(A) {lidalog (LIDA_ERR, A); return -1;}

void lidalog (int, char *);

struct buf_log_t
{
   char *content;
   struct buf_log_t *next;
};

struct buf_log_h
{
   char *content;
   struct buf_log_t *head, *tail;
};

static struct buf_log_h **log_lst;

# define BUF_OFFSET(A,B) A+=(B+proxy_cnt+2)
