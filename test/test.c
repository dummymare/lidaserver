#include<stdio.h>
#include<sys/time.h>
# include <stdlib.h>

#include<unistd.h>
#include <stdio.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
#include <sys/epoll.h>
# include <sys/ipc.h>

struct a
{
   char *x;
   int b;
};

void main()

{
   struct a c;
   c.x = malloc (2048);
   printf ("%d\n", sizeof (c));
}
