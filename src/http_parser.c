# include "global.h"


struct http_request_t *http_parser (char *h, int fd)
{
   char *toklist, *capture, *content_cap, *compat;
   int hash_listcnt;
   struct http_request_t *t = (struct http_request_t *)mem_pool_alloc (sizeof (struct http_request_t), fd);
   size_t size = strlen (h);
   char parse[size];

   /*Protect original data*/
   strncpy (parse, h, size);

   /*Test the method string*/
   t->method = strtok_r (h, " ", &toklist);
   if (method_test (t.method) < 0)
      return NULL;

   /*Get url*/
   t->url = strtok_r (NULL, " ", &toklist);
   if (toklist == NULL)
      return NULL;

   /*Get http version*/
   t->protocal = strtok_r (NULL, "\r\n", &toklist);
   if (protocal_test (t.protocal) < 0)
      return NULL;

   /*Parse the rest string*/
   while (*(toklist+1) != '\n')
   {
      capture = strtok_r (NULL, ": ", &toklist);
      content_cap = strtok_r (NULL, "\n", &toklist);
      
      /*Clear invaild chars*/
      while (*content_cap == ' ')
         content_cap++;
      lida_breakline (content_cap);
      
      hash_set (capture, (void *)content_cap, t, fd, lida_hashkey);

      if (!*(toklist+1))
         return NULL;
   }
   
   toklist+=2;
   /*Here is the request body*/
   t->offset = strlen (toklist);
   
   return (t);
}


int method_test (char *m)
{
   int i;

   for (i=0; i<METHOD_N; i++){
      if (strcmp (m, method_lst[i]) == 0)
         return TRUE;
   }

   return -1;
}


int protocal_test (char *p)
{
   if (strcmp (p, "HTTP/1.1") == 0)
      return TRUE;

   else if (strcmp (p, "HTTP/1.0") == 0)
      return TRUE;

   else return -1;
}


unsigned int lida_hashkey (char *ptr)
{
   unsigned int sum;
   
   while (*ptr)
   {
      sum+=*ptr;
      ptr++;
   }
   
   return (sum%ATTRI_LIST);
}


void hash_conflictset (char *key, void *content_cap, unsigned int cnt, struct http_request_t *t, int fd)
{
   struct http_attrihash *p = t->list[cnt];
   p = p->next;
   
   while (p != NULL)
      p = p->next;
      
   p = (struct http_attrihash *)mem_pool_alloc (sizeof (struct http_attrihash), fd);
   p->content = content_cap;
   p->attri = key;
}


void hash_set (char *key, void *content, struct http_request_t *t, int fd, (*key_set) (char *))
{
   unsigned cnt = (*key_set) (key);
   
   if (t->list[cnt] == NULL)
   {
      t->list[cnt] = (struct http_attrihash *)mem_pool_alloc (sizeof (struct http_attrihash), fd);
      t->list[cnt]->attri = key;
      t->list[cnt]->content = content;
   }
   
   else hash_conflictset (key, content, cnt, t, fd);
}


void *hash_conflictsearch (unsigned int cnt, char *key, struct http_attrihash t[])
{
   struct http_attrihash *p = t[cnt];
   p = p->next;
   
   while (p != NULL)
   {
      if (strcmp (p->attri, key) == 0)
         return (p->content);
      else p++;
   }
   
   return NULL;
}


void *hash_search (char *q, struct http_attrihash t[], (*key_set) (char *))
{
   unsigned int cnt = (*key_set) (q);
   void *ret;
   
   if (t[cnt] == NULL)
      return NULL;
   
   if (strcmp (t[cnt]->attri, q) == 0)
      ret = t[cnt]->content;
      
   else ret = hash_confilctsearch (cnt, q, t);
   
   return ret;
}


unsigned int crc32_port (char *q)
{
   return (crc32(0, (void *)q, strlen (q)) % ATTRI_LIST);
}
