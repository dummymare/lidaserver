# include "global.h"
# include "http_parser.h"
# include "log.h"

char *getime ()
{
   FILE *pipe;
   char ret[30];

   if ((pipe = popen ("date", "r")) == NULL)
      return NULL;

   fgets (ret, 30, pipe);
   lida_breakline (ret);
   fclose (pipe);
   return ret;
}

void lidalog (int type, char *info)
{
   FILE *logfile;
   char *runtime = getime ();
   
   /*Open log file*/
   if ((logfile = fopen ("/var/log/lida/error.log", "a")) == NULL)
      return;

   switch (type)
   {
      case LIDA_ERR:{
         fprintf (logfile, "%d: [%s]   %s :%s\n", (int)getpid (), runtime, info, strerror (errno));
         fclose (logfile);
         break;
      }
      
      case LIDA_ACCESS:{
         fprintf (logfile, "%d: [%s]   %s\n", (int)getpid (), runtime, info);
         fclose (logfile);
         break;
      }
   }
}


void buf_log_init (struct sockaddr_in *opt, int fd)
{
   struct buf_log_h **h = log_lst;
   BUF_OFFSET (h, fd);
   char *time = getime ();
   char *addr = inet_ntoa (opt->s_addr);
   char *header = "======================================================================================\n";
   char *content = mem_pool_alloc (strlen(time)+strlen(addr)+strlen(header)+10, fd);

   sprintf (content, "%sClient:%s[%s]\n", header, addr, time);
   *h = mem_pool_alloc (sizeof (struct buf_log_h), fd);
   (*h)->content = content;
   (*h)->head = mem_pool_alloc (sizeof (struct buf_log_t), fd);
   (*h)->tail = (*h)->head;
}


void request_log (struct http_request_t *h, int fd)
{
   struct buf_log_h **head = log_lst;
   BUF_OFFSET (head, fd);
   char *user_agent = hash_search ("User-Agent", h->list);
   char *host = hash_search ("Host", h->list);
   char *r_len = hash_search ("Content-Length", h->list);
   char *content = mem_pool_alloc (strlen(h->url)+strlen(usr_agent)+strlen(host)+strlen(r_len)+, fd);

   sprintf (content, "Requested:%s\nUrl:%s\nLength:%s\nClient:%s\n\n", host, h->url, r_len, user_agent);
   (*head)->tail->content = content;
   (*head)->tail->next = mem_pool_alloc (sizeof (struct buf_log_h), fd);
}


void err_log (char *str, int fd)
{
   struct buf_log_h **head = log_lst;
   BUF_OFFSET (head, fd);
   char *x = mem_pool_alloc (strlen (str) + 9, fd);

   sprintf (x, "Waring: %s\n");
   (*head)->tail->content = x;
   (*head)->tail->next = mem_pool_alloc (sizeof (struct buf_log_h), fd);
}


void proxy_post_log ()
{

}


void reponse_log (struct http_response_h *h, int fd)
{
   
}
