# include "main.h"
# include "ghost.h"

# define HUGE_THREAD_POOL 

pthread_mutex_t thread_mutex;
pthread_key_t thread_key;
pthread_once_t init_done = PTHREAD_ONCE_INIT;

jmp_buf thread_env;
jmp_buf thread_init_env;


int thread_pool_init (int cnt, struct thread_pool_h *ret)
{
   struct queue_chain_ctl *t = malloc (sizeof (struct queue_chain_ctl));
   t->head = NULL;
   t->tail = NULL;
   ret->h = t;

   struct thread_data_t *d;
   unsigned pool_flag;
   struct mem_cache_head *pool, *spool;
   int err;

   /*Init memory caches*/
   if (cnt > HUGE_THREAD_POOL){
      mem_cache_creat (sizeof (struct queue_chain_t), &spool);
      mem_cache_creat (sizeof (struct thread_data_t), &pool);

      ret->pool = pool;
      ret->spool = spool;

      pool_flag = 1;
   }
   else pool_flag = 0;

   /*Init thread mutex*/
   if (err = pthread_mutex_init (&thread_mutex, NULL)){
      lidalog (LIDA_ACCESS, strerror (err));
      return -1;
   }

   while (cnt--)
   {
      d = pool_flag?malloc (sizeof (struct thread_data_t)):mem_cache_alloc (pool);
      d->tmp_flag = 0;
      d->pool = pool_flag?pool: NULL;

      if (err = pthread_create (&(d->id), NULL, thread_trigger, (void *)d)){
         lidalog (LIDA_ACCESS, strerror (err));
	 goto ERROR;
      }
      
      queue_chain_in ((void *)d, t, spool);
   }
   return 0;

ERROR:
   char msg[30];

   sprintf (msg, "Not enough thread created, rested %d", cnt);
   lidalog (LIDA_ACCESS, msg);
   return -1;
}


void thread_key_init ()
{
   int err;

   while (err = pthread_key_create (&thread_key, NULL)){
      /*Wait for avalible resource*/
      if (err == EAGAIN)
         continue;
      else {
         lidalog (LIDA_ACCESS, "Memory error");
	 pthread_exit ((void *)-1);
      }
   }
}


void *thread_trigger (void *d)
{
   int err;
   jmp_buf *p_env;

   /*Select signal mask*/
   struct sigset_t sigset;

   sigemptyset (&sigset);
   sigaddset (&sigset, SIGALRM);
   sigaddset (&sigset, SIGUSR1);
   sigaddset (&sigset, SIGUSR2);
   sigaddset (&sigset, SIGINT);
   sigaddset (&sigset, SIGCONT);
   sigaddset (&sigset, SIGCHLD);
   
   if (err = pthread_sigmask (SIG_BLOCK, &sigset, NULL)){
      lidalog (LIDA_ACCESS, strerror (err));
      pthread_exit ((void *)-1);
   }

   /*Set up signal handler*/
   struct sigaction act;

   act.sa_handler = thread_reset;

   if (sigaction (SIGHUP, &act, NULL) < 0){
      lidalog (LIDA_ERR, "sigaction");
      pthread_exit ((void *)-1);
   }

   /*Active thread data key*/
   pthread_once (&init_done, thread_key_init);

   while (err = pthread_setspecific (thread_key, (const void *)&thread_env)){
      if (err == ENOMEM)
         continue;
      else {
         lidalog (LIDA_ACCESS, "thread key error exiting");
	 pthread_exit ((void *)-1);
      }
   }

   if ((p_env = (jmp_buf *)pthread_getspecific (thread_key)) == NULL){
      err_log (LIDA_ACCESS, "Request cannot be handled due to thread jmp_buf error", fd);
      pthread_exit ((void *)-1);
   }

   /*Restart here*/
   setjmp (*p_env);

   /*Wait for action signals*/
   int sig;
   struct sigset_t wait_mask;

   sigemptyset (&wait_mask);
   sigaddset (&wait_mask, SIGCONT);
   sigaddset (&wait_mask, SIGINT);

   while (!(err = sigwait (&wait_mask, &sig))){
      switch (sig)
      {
         case SIGCONT: goto HANDLE;
	 case SIGINT: pthread_exit ((void *)-1);
	 default: continue;
      }
   }

   lidalog (LIDA_ACCESS, strerror (err));
   pthread_exit ((void *)-1);

HANDLE:
   switch ()
}


void thread_reset (int signo)
{
   jmp_buf *p_env = (jmp_buf *)pthread_getspecific (thread_key);

   if (p_env == NULL){
      lidalog (LIDA_ACCESS, "thread cannot reset");
      pthread_exit ((void *)-1);
   }

   longjmp (*p_env, 1);
}


pthread_t thread_pool_alloc (struct thread_pool_h *pool, 
                             int fd, int pfd, void *msg,
			     FILE *fp, unsigned type)
{
   int err, tmp_thread_port (int, int, void *, FILE *, unsigned);
   pthread_t thid;

   setjmp (thread_init_env);

START:
   struct thread_data_t *d = queue_chain_out (pool->h, pool->spool);
   
   /*We can create a tmporiry thread here*/
   if (d == NULL){
      if (thid = tmp_thread_port (fd, pfd, msg, fp, proxy))
         goto FAILED;
      else return thid;
   }

   else {
      d->fd = fd;
      d->pfd = pfd;
      d->data = msg;
      d->fp = fp;
      d->type = type;
      d->pool = pool->pool;
   }
   
   /*Wake up handler thread*/
   if (err = pthread_kill (d->id, SIGCONT)){
      if (err == ESRCH){
         if (d->pool == NULL)
	    free (d);
         else mem_cache_free (d, d->pool);

	 goto START;
      }
      else goto FAILED;
   }

   return (d->id);

FAILED:
   err_log (LIDA_ACCESS, "Failed to handle current request due to thread problems", fd);
   error_post (fd);
   return -1;
}


pthread_t tmp_thread_port (int fd, int pfd, void *msg, FILE *fp, unsigned type)
{
   int err;
   struct thread_data_t *d = malloc (sizeof (struct thread_data_t));

   d->fd = fd;
   d->pfd = pfd;
   d->data = msg;
   d->fp = fp;
   d->type = type;
   d->pool = NULL;
   d->tmp_flag = 1;

   if (err = pthread_create (&(d->id), NULL, thread_trigger, (void *)d)){
      /*Re alloc from thread pool due to system limit*/
      if (err == EAGAIN)
         longjmp (thread_init_env, 1);
      else return -1;
   }

   return (d->id);
}
