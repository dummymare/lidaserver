# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <error.h>
# include <string.h>
# include <strings.h>
# include <setjmp.h>
# include <errno.h>

# include <sys/types.h>
# include <sys/signal.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <sys/wait.h>
# include <sys/errno.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <dlfcn.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# define ATTRI_LIST 29
int hostnum;
