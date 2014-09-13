# include "global.h"
# include "ghost.h"

void http_parser (char *);

void http_GET (char *);
void http_POST (char *);

void func_redirect (int ssock)
{
   int buflen = 0;
   char *netbuf = (char *)malloc (4096);
   char *docname, *request_form;
   
   int is_keepalive (void);
   void keepalive_exit (int);
   char *lida_post_file (int, struct stat *, void *);
   
   if (signal (SIGALRM, keepalive_exit) < 0)
   {
      log_argv[0] = "signal";
      lidalog (LIDA_ERR, log_argv);
      exit (1);
   }


G_START:
   buflen = read (ssock, (void *)netbuf, 4096);
   alarm (0);
      
   if (buflen < 0)
   {
      log_argv[0] = "unknown"; log_argv[1] = (char *)malloc (15);
      strcpy (log_argv[1], inet_ntoa (client.sin_addr));
      log_argv[2] = "Cannot read httphead from client";
      lidalog (LIDA_ACCESS, log_argv);
      exit (1);  
   }
   
   
   if (setjmp (ghost_dump) != 0)
      exit (1);
   http_parser (netbuf);
   
   
   
   if (setjmp (ghost_dump) != 0)
      exit (1);
   char *toklst;
   if (strcmp (request_headline.method, "GET") == 0)
   {
      docname = strtok_r (request_headline.url, "?", &toklst);
      request_form = toklst;
   
      http_GET (docname, request_form);
   }
   
   
   else if (strcmp (request_headline.method, "POST") == 0)
   {
      docname = request_headline.url;
      request_form = form_data;
      
      http_POST (docname, request_form);
   }
   
   if (is_keepalive ())
   {
      alarm (8);
      goto G_START;
   }
   
   exit (0);
}


int is_keepalive ()
{
   int cnt = lida_hashkey ("Connection");
   char *tmp;
   
   if (strcmp (lida_hashlist[cnt]->attri, "Connection") == 0){
      if (strcmp (lida_hashlist[cnt]->content, "keep-alive") == 0)
         return TRUE;
      else return FALSE;
   }
   
   else tmp = hash_conflictsearch (cnt, "Host");
   
   if (strcmp (tmp, "keep-alive") == 0)
      return TRUE;
   else return FALSE;
}


void keepalive_exit (int signo)
{
   log_argv[0] = request_host->hostname;
   log_argv[1] = (char *)malloc (15);
   strcpy (log_argv[1], inet_ntoa (client.sin_addr));
   log_argv[2] = "keep-alive out of time";
   lidalog (LIDA_ACCESS, log_argv);
   
   exit (1);
}
