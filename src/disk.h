struct disk_pool_head
{
   void *start, *end;
   struct disk_pool_head *next;
};

static struct disk_pool_head **dpool_lst;
