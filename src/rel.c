void url_parser (char *url, char *path, struct module_function *handler, char *qform)
{
   char *toklst, *path_tmp, *p;
   
      
   path_tmp = strtok_r (url, "?", &toklst);
   
   if (strcmp (path_tmp, "/") == 0)
      path = request_host->frontpage;
   
   else path = path_tmp;
      
   qform = toklst;
   p = path_tmp;
   
   while (*p)
      p++;
      
   while (*p != '/')
     p--;
     
   p++;
   handler = lida_regexp (p);
}



struct module_function *lida_regexp (char *ptr)
{
   struct module_function *p;
   
   for (p = module_head->next; p != NULL; p = p->next)
      if (lida_match (p->match_exp, ptr))
         return p;
         
         
   return NULL;
}



int lida_match (char *exp, char *ptr)
{
   int rst[2];
   
   char *p, *toklst;
   char tarexp[20];
   strcpy (tarexp, exp);
   
  brastack.cnt = 0;
  braqueue.front = 0;
  braqueue.tail = 0;
   
   p = strtok_r (tarexp, "^", &toklst);
   if (p != NULL) {
      while (*p)
      {
         switch (*p)
         {
            case '!': neg_flag = 1; break;
            
            case '~': {
               if (neg_flag) {
                  cap_flag = 0;
                  neg_flag = 0;
                  break;
               }
               
               else cap_flag = 1; break;
            }
            
            case ('(' | ')' | '[' | ']' | '*'): {
               braqueue.ptr[braqueue.front] = p
               braqueue.front++;
               break;
            }
         }
      }
      
      rst[0] = front_match (exp, ptr);
   }
   
   else rst[0] = 1;
      
   p = strtok_r (NULL, "$", &toklst);
   if (p != NULL) {
      while (*p)
      {
         switch (*p)
         {
            case '!': neg_flag = 1; break;
            
            case '~': {
               if (neg_flag) {
                  cap_flag = 0;
                  neg_flag = 0;
                  break;
               }
               
               else cap_flag = 1; break;
            }
            
            case ('(' | ')' | '[' | ']' | '*'): {
               brastack.ptr[brastack.cnt] = p;
               brastack.cnt++;
               break;
            }
         }
      }
      
      while (*ptr)
         ptr++;
      
      rst[1] = tail_match (p-1, ptr-1);
   }
   
   
   
   if (rst[0] && rst[1])
      return TRUE;
   else return FALSE;
}



int isletter (char x)
{
   if ((x<'A') || ((x>'Z') && (x<'a')) || (x>'z'))
      return FALSE;
      
   else return TRUE;
}



int strcmp_noncap (char *str1, char *str2)
{
   const unsigned char *s1 = (const unsigned char *)str1;
   const unsigned char *s2 = (const unsigned char *)str2;
   int delta = 0;
   
   while (*s1 || *s2)
   {
      delta = *s2 - *s1;
      
      if (delta) {
         if (isletter (*s1) && isletter (*s2)) {
            delta = *s2-0x20-*s1;
            
            if (delta)
               delta = *s2+0x20-*s1;
               
            if (delta)
               return delta;
         }
         
         else return delta;
      }
      
      s1++;
      s2++;
   }
   
   return 0;
}


int strncmp_rev (char *str1, char *str2, int maxlen)
{
   while (*str1)
      str1++;
   
   const unsigned char *s1 = (const unsigned char *)str1;
   const unsigned char *s2 = (const unsigned char *)str2;
   int delta = 0;
   
   while (maxlen--) {
      delta = *s2 - *s1;
      
      if (delta) {
         if (cap_flag)
            return delta;
         if (isletter (*s1) && isletter (*s2)) {
            delta = *s2-0x20-*s1;
            
            if (delta)
               delta = *s2+0x20-*s1;
               
            if (delta)
               return delta;
         }
         
         else return delta;
      }
         
      s1--;
      s2--;
   }
   
   return 0;
}



int front_match (char *exp, char *ptr)
{
   char cmp1[20], cmp2[20], submatch[20], block_cap[20];
   char *p = exp, *tmp;
   int cnt = 0, subcnt = 0;
   int list_match (char *, char *);
   
   while (braqueue.front != braqueue.tail)
   {
      cnt = braqueue.ptr[braqueue.tail] - p - 1;
      
      strncpy (cmp1, p, cnt);
      strncpy (cmp2, ptr, cnt);
      
      if (cap_flag)
         if (strcmp (cmp1, cmp2) != 0)
            return FALSE;
      else if (strcmp_noncap (cmp1, cmp2) != 0)
         return FALSE;
         
      p+=cnt;
      ptr+=cnt;
      
      switch (*(braqueue.ptr[braqueue.tail]))
      {
         case '(': {
            tmp = braqueue.ptr[braqueue.tail];
            while (*(braqueue.ptr[braqueue.tail]) != ')')
               braqueue.tail++;
               
            subcnt = braqueue.ptr[braqueue.tail] - tmp - 2;
            strncpy (submatch, tmp+1, subcnt);
            strcpy (block_cap, ptr+1);
            
            if (!lida_match (submatch, block_cap))
               return FALSE;
            
            ptr+=(subcnt+2);
            break;
         }
         
         case '[': {
            tmp = braqueue.ptr[braqueue.tail];
            while (*(braqueue.ptr[braqueue.tail]) != ']')
               braqueue.tail++;
               
            subcnt = braqueue.ptr[braqueue.tail] - tmp - 2;
            strncpy (submatch, tmp+1, subcnt);
            strncpy (block_cap, ptr+2, subcnt);
            
            if (!list_match (submtch, block_cap))
               return FALSE;
               
            ptr+=(subcnt+2);
            break;
         }
         
         case '*': {
            subcnt = braqueue.ptr[braqueue.tail+1] - braqueue.ptr[braqueue.tail] - 2;
            strncpy (submatch, braqueue.ptr[braqueue.tail]+1, subcnt);
            
            ptr++;
            if (strncmp_rev (submatch, ptr, subcnt) != 0)
               return FALSE;
         }
      }
      
      braqueue.tail++;
   }
}



int tail_match (char *exp, char *ptr)
{
   char cmp1[20], cmp2[20], submatch[20], block_cap[20];
   char *p = exp, *tmp;
   int cnt = 0, subcnt = 0;
   int list_match (char *, char *);
   
   while (*(brastack.ptr[brastack.cnt]))
   {
      cnt = p - brastack.ptr[brastack.cnt] - 1;
      
      strncpy (cmp1, p-cnt, cnt);
      strncpy (cmp2, ptr-cnt, cnt);
      
      if (cap_flag)
         if (strcmp (cmp1, cmp2) != 0)
            return FALSE;
      else if (strcmp_noncap (cmp1, cmp2) != 0)
         return FALSE;
         
      p-=cnt;
      ptr-=cnt;
      
      switch (*(brastack.ptr[brastack.cnt]))
      {
         case ')': {
            tmp = brastack.ptr[braqueue.tail];
            while (*(brastack.ptr[brastack.cnt]) != '(')
               brastack.cnt--;
               
            subcnt = tmp - brastack.ptr[brastack.cnt] - 2;
            strncpy (submatch, tmp-subcnt-1, subcnt);
            strncpy (block_cap, ptr-subcnt-2, subcnt);
            
            if (!lida_match (submatch, block_cap))
               return FALSE;
               
            ptr-=(subcnt+2);
            break;
         }
         
         case ']': {
            tmp = brastack.ptr[braqueue.tail];
            while (*(brastack.ptr[brastack.cnt]) != '[')
               brastack.cnt--;
               
            subcnt = tmp - brastack.ptr[brastack.cnt] - 2;
            strncpy (submatch, tmp-subcnt-1, subcnt);
            strncpy (block_cap, ptr-subcnt-2, subcnt);
            
            if (!list_match (submatch, block_cap))
               return FALSE;
               
            ptr-=(subcnt+2);
            break;
         }
         
         case '*': {
            subcnt = brastack.ptr[brastack.cnt-1] - brastack.ptr[brastack.cnt] - 2;
            strncpy (submatch, brastack.ptr[brastack.cnt-1]-1, subcnt);
            
            ptr-=(subcnt+2);
            if (strcmp (submatch, ))
         }
      }
   }
}
