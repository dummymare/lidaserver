# include "global.h"
# include "ghost.h"

# include <sys/mman.h>
# include <sys/stat.h>
# include <time.h>

# define HTTP_OK 200
# define HTTP_NOTFOUND 404


static char content_type[20];
char *server_name = "LIDA/1.0";

char *lida_post_file (int, struct stat *, void *);

int lida_lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
   struct flock lock;
   
   lock.l_type = type;
   lock.l_start = offset;
   lock.l_whence = whence;
   lock.l_len = len;
   
   return (fcntl (fd, cmd, &lock));
}


void write_exit (int signo)
{
   log_argv[0] = request_host->hostname; log_argv[1] = inet_ntoa (client.sin_addr);
   log_argv[2] = "Posting data to client out of time";
   lidalog (LIDA_ACCESS, log_argv);
   
   longjmp (ghost_dump, 1);
}


void http_GET (char *docname, char *request_form)
{
   char path[30], *tmp;
   int fd;
   
   struct  stat statbuf;
   struct config_info *host_search (void);
   struct module_function *module_search (char *, char *);
   
   if ((request_host = host_search ()) == NULL)
   {
      log_argv[0] = "unknown"; log_argv[1] = (char *)malloc (15);
      strcpy (log_argv[1], inet_ntoa (client.sin_addr));
      log_argv[2] = "Requested host do not exit";
      lidalog (LIDA_ACCESS, log_argv);
      
      char *str = lida_post_file (404, NULL, NULL);
      write (ssock, str, strlen (str));
      exit (1);
   }
   
   
   if (chdir (request_host->path) < 0)
   {
      log_argv[0] = "chdir";
      lidalog (LIDA_ERR, log_argv);
      exit (1);
   }
   
   /*common modules code*/

   if (strcmp (docname, "/") == 0)
      strcpy (path, request_host->frontpage);

   else {
      *path = '.';
      tmp = path;
      tmp++;
      strcpy (tmp, docname);
   }

   if ((fd = open (path, O_RDONLY)) < 0)
   {
      log_argv[0] = "open";
      lidalog (LIDA_ERR, log_argv);
      longjmp (ghost_dump, 1);
   }
   
   if (lida_lock_reg (fd, F_GETLK, F_RDLCK, 0, SEEK_SET, 0) < 0)
   {
      log_argv[0] = "fcntl";
      lidalog (LIDA_ERR, log_argv);
      longjmp (ghost_dump, 1);
   }

   if (fstat (fd, &statbuf) < 0)
   {
      log_argv[0] = "fstat";
      lidalog (LIDA_ERR, log_argv);
      longjmp (ghost_dump, 1);
   }

   if ((fdoc = mmap (0, statbuf.st_size, PROT_READ,
                     MAP_PRIVATE, fd, 0)) == MAP_FAILED)
   {
      log_argv[0] = "mmap";
      lidalog (LIDA_ERR, log_argv);
      longjmp (ghost_dump, 1);
   }
   
   
   get_response_info (content_type, path);
   
   if (signal (SIGALRM, write_exit) < 0)
   {
      log_argv[0] = "signal";
      lidalog (LIDA_ERR, log_argv);
      longjmp (ghost_dump, 1);
   }
   
   alarm (10);
   
   lida_post_file (HTTP_OK, &statbuf, fdoc);
   
   /*Add some code to handle request content*/
   
   alarm (0);
}



char *lida_post_file (int status, struct stat *statbuf, void *content)
{
   char reponse_head[2048], date[30];
   char *p = reponse_head;
   
   FILE *pipe = popen ("date", "r");
   fgets (date, 30, pipe);
   fclose (pipe);
 

   switch (status)
   {
      case HTTP_NOTFOUND :{
         sprintf (p, "HTTP/1.1 404 ERROR\r\n");
         
         while (*(++p));
         
         sprintf (p, "Date: %s\r", date);
         
         while (*(++p));
            
         sprintf (p, "Server: %s\r\n", server_name);
         
         while (*(++p));
            
         sprintf (p, "Content-Length: 0\r\n\r\n");
         
         return reponse_head;
         break;
      }
      
      case HTTP_OK:{
         sprintf (p, "HTTP/1.1 200 OK\n\r");
         
         while (*(++p));
            
         sprintf (p, "Server: %s\n\r", server_name);
         
         while (*(++p));
            
         sprintf (p, "Date: %s\r", date);
         
         while (*(++p));
         
         sprintf (p, "Content-type: %s\n\r", content_type);
         
         while (*(++p));
         
         sprintf (p, "Last-modified: %s\n\r", ctime (&statbuf->st_mtime));
         
         while (*(++p));
         
         sprintf (p, "Content-length: %d\n\r\n\r", strlen (content));
         
         if (write (ssock, (void *)reponse_head, strlen (reponse_head)) < 0)
         {
            log_argv[0] = "write";
            lidalog (LIDA_ERR, log_argv);
            longjmp (ghost_dump, 1);
         }
         
         if (write (ssock, content, strlen (content)) < 0)
         {
            log_argv[0] = "write";
            lidalog (LIDA_ERR, log_argv);
            longjmp (ghost_dump, 1);
         }
      }
   }
}



void http_POST (char *docname, char *request_form)
{
   return;
}


struct config_info *host_search (void)
{
   int i, cnt = lida_hashkey ("Host");
   char *host_name;
   struct config_info *p = httphost;
   
   
   if (strcmp (lida_hashlist[cnt]->attri, "Host") == 0)
      host_name = lida_hashlist[cnt]->content;
      
   else host_name = hash_conflictsearch (cnt, "Host");
   
   for (i=0; i<hostnum; i++)
   {
      if (strcmp (p->hostname, host_name) == 0)
         return p;
      
      else p++;
   }
   
   return NULL;
}
