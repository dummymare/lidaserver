# include "heap.h"


int heap_init (int size, struct heap_h *head, int fd)
{
   head->size = ++size;
   head->cnt = 0;

   if ((head->p = fd?mem_pool_alloc (sizeof (struct heap_t *) * size, fd): calloc (size, sizeof (struct heap_t *))) == NULL)
      return -1;

   return 0;
}


int heap_insert (void *data, unsigned int key, struct heap_h *h)
{
   int i;

   if (HEAP_FULL (h))
      return -1;

   i = ++(h->cnt);
   while ((i != 1) && (key > (h->p + i/2)->key))
   {
      memcpy (h->p + i, h->p + i/2, sizeof (struct heap_t));
      i /= 2;
   }

   (h->p + i)->key = key;
   (h->p + i)->data = data;
}


struct heap_t *heap_max_delete (struct heap_h *h)
{
   int parent, child;
   struct heap_t *item, *temp;

   if (HEAP_EMPTY (h))
      return NULL;

   item = (h->p + 1);
   temp = (h->p + (h->cnt)--);

   parent = 1;
   child = 2;

   while (child <= h->cnt)
   {
      if ((child < h->cnt) && ((h->p + child)->key < (h->p + child + 1)->key))
         child++;
      
      if (temp->key >= (h->p + child)->key)
         break;

      memcpy (h->p + parent, h->p + child, sizeof (struct heap_t));
      parent = child;
      child *= 2;
   }

   memcpy (h->p + parent, temp, sizeof (struct heap_t));
   return item;
}
