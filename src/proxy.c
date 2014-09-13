void post_to_proxy (char *url, struct config_info *q, int fd, void *h, FILE *fp)
{
   FILE *pipe;
   struct proxy_pass *t;

   for (t=q->active; t!=NULL; t++){
      char ret[3], r[strlen(url) + strlen(t->match) + 11];
      sprintf (r, "./match.pl %s %s", url, t->match);

      if ((pipe = popen (r, "r")) == NULL){
         err_log (LIDA_ERR, "popen", fd);
	 continue;
      }

      fgets (ret, 3, pipe);
      fclose (pipe);

      if (strcmp (ret, "yes"))
         continue;
      else goto F_CON;
   }

   error_post (fd);
   return;

F_CON:
   struct proxy_node *g;
   int pfd = 0;

   /*Get pthread lock to alloc proxy fd*/
   pthread_mutex_lock(&pth_lock);
   pfd = load_balance (t->proxy, &g);
   pthread_mutex_unlock (&pth_lock);
   
   /*Send first 8k data*/
   size_t c, len;
   void *s;

SND_START:
   for (c=0, len=8192, s=h; len; s+=c, len-=c){
      c = write (pfd, s, len);

      if (c < 0)
      {
         switch (errno)
	 {
	    case EBADF:
	    case EDESTADDRREQ: {
	       if ((pfd = re_connect (g, pfd, fd)) < 0){
	          proxy_node_del (g);

		  char err_msg[30];
		  sprintf (err_msg, "A node server is down: %s:%d", g->ip, g->port);
		  lidalog (LIDA_ACCESS, err_msg);

	          goto F_CON;
	       }
               /*We need to reset the connect*/
	       else goto SND_START;
	    }

	    default: goto SND_FAIL;
	 }
      }
   }

   /*Send disk buffer*/
   if (fp == NULL)
      return;

   char buf[4096];
   size_t buf_siz = 0, rd_siz = 0;
   void *ptr;

   rewind (fp);
   while (!feof (fp))
   {
      rd_siz = fread (buf, 4096, 1, fp);
      ptr = buf;

      while ((buf_siz = write (pfd, ptr, rd_siz)) >= 0)
         ptr+=buf_siz;
      
      if (buf_siz < 0)
         goto SND_FAIL;
   }
   
   /*Add the proxy-pass event to epoll queue*/
   struct epoll_event m;
   m.events = EPOLLIN | EPOLLRDHUP;
   m.data.u64 = pfd;

   if (epoll_ctl (epfd, EPOLL_CTL_ADD, &m) < 0)
   {
      err_log (LIDA_ERR, "epoll_ctl", fd);
      goto SND_FAIL;
   }

   return;

SND_FAIL:
   err_log (LIDA_ACCESS, "server not avalible", fd);
   error_post (fd);

   if (pfd)
   {
      pthread_mutex_lock(&pth_lock);
      ret_proxy_fd (pfd, &g);
      pthread_mutex_unlock (&pth_lock);
   }
}


int slaveTCP (char *ip, short port)
{
   int sock;
   struct sockaddr_in option;

   if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
      ERR_RET ("socket");

   option.sin_family = AF_INET;
   option.sin_port = htons (port);

   if ((option.sin_addr.s_addr = inet_addr (ip)) == INADDR_NONE)
   {
      char e[30];
      sprintf (e, "Proxy ip invaild: %s", ip);
      lidalog (LIDA_ACCESS, e);
      return ERROR;
   }

   if (connect (sock, (struct sockaddr *)&option, sizeof (struct sockaddr_in)) < 0)
      ERR_RET ("connect");

   return sock;
}


int proxy_init ()
{
   int i, ret = 0;
   unsigned long cnt;
   struct config_info *n;
   struct proxy_pass *l;
   struct proxy_node *p;

   for (n=httphost; n!=NULL; n++)
   {
      for (l=n->active; l!=NULL; l++)
      {
         for (p=l->proxy->node; p!=NULL; p = p->next)
	 {
	    p->fd = calloc (p->weight, sizeof (int));

	    for (i=0; i<p->weight; i++){
	       if ((*(p->fd + i) = slaveTCP (p->ip, p->port)) < 0)
	          break;
	       /*Set pfd to non-block mode*/
	       else{
	          if (SET_NON_BLOCK (*(p->fd + i)) < 0){
		     lidalog (LIDA_ERR, "fcntl");
		     close (*(p->fd + i));
		  }
	       }
	    }

	    ret+=i;
	    p->weight = i;

	    if (p->weight <= 64){
	       for (cnt=1, i=0; i<p->weight; i++, cnt<<1)
	          p->status.s_stat |= cnt;
	    }

	    else {
	       int f_cnt = (int)(p->weight/8) + (p->weight%8)?1:0;
	       p->status.addr = malloc (f_cnt);

               for (i=0; i<f_cnt; i++)
	          *(p->status.addr + i) = 0xff;

	       if (i = p->weight % 8)
	       {
	          unsigned s_cnt;
		  *(p->status.addr + f_cnt - 1) = 0;

	          for (s_cnt=1; i--; s_cnt<<1)
		     *(p->status.addr+f_cnt-1) |= s_cnt;
	       }
	    }
	 }
      }
   }

   return ret;
}


int re_connect (struct proxy_node *p, int pfd, int fd)
{
   struct sockaddr_in opt;
   char e[30];

   opt.sin_family = AF_INET;
   opt.sin_port = htons (p->port);

   if ((op.sin_addr.s_addr = inet_addr (p->ip)) == INADDR_NONE)
   {
      sprintf (e, "Proxy ip invaild: %s", p->ip);
      err_log (LIDA_ACCESS, e, fd);
      return -1;
   }
   
   /*If error occurs, try 5 times*/
   int cnt = 5;
   
   while (connect (pfd, (struct sockaddr *)&opt, sizeof (struct sockaddr_in)) < 0 && cnt--)
      err_log (LIDA_ERR, "connect", fd);

   if (cnt)
      return pfd;

   else return -1;
}


int load_balance (struct proxy_group *g, struct proxy_node **p)
{
   struct proxy_node *n = g->proxy, *t;
   int cnt;

   for (t=n->next; n!=NULL; t=(t->key < n->key)?t:n, n=n->next);
   *p = t;

   if (t->weight <= 64)
   {
      unsigned long a;

   /*Keep scanning until a thread return */
   ST_SCAN1:
      for (a=1, cnt=0; cnt<t->weight; a<<1, cnt++){
         if (a & t->status.s_stat){
	    t->status.s_stat &= ~a;
	    t->act_cnt--;
	    t->key = t->act_cnt/t->weight;
	    return (*(t->fd + cnt));
	 }
      }

      goto ST_SCAN1;
   }

   else {
      unsigned b;
      void *c;
   
   ST_SCAN2:
      for (c=t->status.addr; c!=NULL; c++)
      {
         for (b=1, cnt=0; cnt<8; b<<1, cnt++){
	    if (b & *c){
	       *c &= ~b;
	       t->act_cnt--;
	       t->key = t->act_cnt/t->weight;
	       return (*(t->fd + (void *)c - (void *)t->status.addr) + cnt);
	    }
	 }
      }
      
      goto ST_SCAN2;
   }
}


void proxy_node_del (struct proxy_node *n)
{
   n->prev->next = n->next;
   n->next->prev = n->prev;
   free (n);
}
