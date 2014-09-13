# include "main.h"
# include "listener.h"
# include "http_parser.h"
# include "log.h"

void post_file (char *type, char *method, char *path, int fd)
{
   FILE *fp = fopen (path, "r");

   if (fp == NULL){
      err_post (fd);
      return;
   }

   struct stat statbuf;
   if (stat (path, &statbuf) < 0){
      err_post (fd);
      return;
   }

   struct http_response_h s;
   s.version = "HTTP/1.1";
   s.status = "200 OK";
   s.content_type = type;
   sprintf (s.content_length, "%ld", statbuf.st_size);
   s.date = getime ();
   s.mtime = ctime (statbuf.st_mtime);

   if (post_header (&s, fd) < 0)
      return;

   if (strcmp (method, "HEAD") == 0)
      return;
   
   char buf[PAGE_SIZ];
   size_t buf_siz;

   fcntl (fd, F_SETFD, fcntl (fd, F_GETFD, 0) & ~O_NONBLOCK);

   while ((buf_siz = fread (buf, PAGE_SIZ, 1, fp)) > 0)
      write (fd, buf, buf_siz);

   fcntl (fd, F_SETFD, fcntl (fd, F_GETFD, 0) | O_NONBLOCK);
}


int post_header (struct http_response_h *r, int fd)
{
   size_t len = strlen (r->version) + strlen(r->status) +
   strlen (r->content_type) + strlen (r->content_length) +
   strlen (r->date) + strlen (r->mtime) + 82;

   char header[len];

   sprintf (header, "%s %s\r\n
                     Server: LIDA/1.0\r\n
                     Content-Type: %s\r\n
		     Content-Length: %s\r\n
		     Date: %s\r\n
		     Last-Modified: %s\r\n
		     \r\n", r->version, r->status, r->type, r->date,
		     r->mtime);

   if (write (fd, header, len) < 0)
      return -1;

   return 1;
}
