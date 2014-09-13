# include "timer.h"


struct timer_h *timer_init ()
{
   struct timer_h *p = malloc (sizeof (struct timer_h));

   p->max = true_type.connect;
   p->cnt = 0;
   p->s = NULL;

   mem_cache_create (sizeof (struct timer_t), &(p->pool), p->max);
   mem_cache_create (sizeof (struct rbtree_t), &(p->rbt_pool), p->max);

   return p;
}


void timer_insert (struct timer_h *h, int fd, int pfd)
{
   /*Max connect reached*/
   if (h->max == h->cnt){
      lidalog (LIDA_ACCESS, "Timer is full");
      return;
   }

   struct timer_t *p = mem_cache_alloc (h->pool);
   char msg[30];

   p->fd = fd;
   p->pfd = pfd;
   p->time = time (&(p->time));

   if (rbtree_insert (&(h->s), fd, (void *)p, h->rbt_pool, RBT_TIMER) < 0){
      sprintf (msg, "Failed to put %d into timer", fd);
      lidalog (LIDA_ACCESS, msg);
   }

   h->cnt++;
}


void timer_search (struct timer_h *h, int fd, void (*handler) (struct timer_t *))
{
   char msg[30];
   struct timer_t *d = rbtree_search (fd, h->s, RBT_SEARCH);

   if (d == NULL){
      sprintf (msg, "Cannot find %d, operation failed", fd);
      lidalog (LIDA_ACCESS, msg);
      return;
   }

   (*handler) (d);
}


void timer_cleaner (void *d, struct mem_cache_head *pool)
{
   /*It doesn't matter if this action succeed*/
   pthread_kill ((struct timer_t *)d->thread_id, SIGHUP);

   int fd = (struct timer_t *)d->fd;
   mem_pool_destroy (fd);
   log_sync (fd);

   if (pool == NULL)
      free (d);
   else mem_cache_free (d, pool);
}


void timer_delete (struct timer_h *h, int fd)
{
   rbtree_delete (fd, h->s, timer_cleaner, h->pool, h->rbt_pool);
   h->cnt--;
}


int timer_check (void *data, struct mem_cache_head *pool)
{
   time_t cur, tmp = (struct timer_t *)data->time;

   cur = time (&cur);
   
   if ((cur - tmp) >= true_type.keep_alive){
      timer_cleaner (data, pool);
      return 1;
   }
   else return 0;
}


void timer_task (struct timer_h *h)
{
   struct stack_t *tape;

   stack_init (&tape, ture_type.connect);

   rbtree_ergodic (h->s, timer_check, h->pool, tape);
   retree_stack_del (tape, h->s, timer_cleaner, h->pool, h->rbt_pool);

   free (tape);
}
