# include "main.h"
# include "disk.h"

struct disk_pool_head *disk_pool_creat (int fd)
{
   
   struct disk_pool_head t, **h = dpool_lst;
   POOL_OFFSET (h, fd);

   if (disk_alloc (fd))
}
