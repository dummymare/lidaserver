# include "ghost.h"
# include "global.h"
# include "listener.h"
# include "cache.h"
# include "main.h"
# include "log.h"


void backend ()
{
   pid_t sid;
   int proc_info;

   /*Init daemon process*/
   umask (0);
   
   if ((sid = setsid ()) < 0)
      ERR_EXIT ("setsid");
   
   if (chdir ("/") < 0)
      ERR_EXIT ("chdir");
   
   close(STDIN_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);
   
   /*Init child processes*/
   lida_fork ();
   
   /*Register signal handlers*/
   if (signal (SIGCHLD, check_client) == SIG_ERR)
      ERR_EXIT ("signal");
   
   if (signal (SIGINT, read_exit) == SIG_ERR)
      ERR_EXIT ("signal");
      
   if (signal (SIGUSR1, lida_reconfig) == SIG_ERR)
      ERR_EXIT ("signal");
       
   if (signal (SIGALRM, lida_timer_action) ==SIG_ERR)
      ERR_EXIT ("signal");
   
   /*Enter loop to get signals*/
   while (1);
}



void lida_fork ()
{
   int i, pid;
   int m = getpid ();
   
   listener_lst = (int *)malloc (true_type.listener, sizeof (int));

   /*Set up listen port*/
   if ((msock = passiveTCP ()) < 0)
      exit (1);
      
   /*Start file cache process*/
   pid = fork ();
   
   if (pid < 0)
      ERR_EXIT ("fork");
      
   else if (pid == 0)
      lida_cache_proc (m);
   /*Save cache pid for listeners*/
   else cache_pid = pid;
   
   /*Start listener process*/
   for (i=0; i<true_type.listener; i++)
   {
      pid = fork ();
      
      if (pid < 0)
         ERR_EXIT ("fork");
         
      else if (pid == 0)
         listener_proc (msock, m, cache_pid);
         
      /*Record the listeners pids*/
      else *(listener_lst+i) = pid;
   }
}


int passiveTCP ()
{  
   int socket_fd;
   struct sockaddr_in options;
   
   if ((socket_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
   {
      lidalog (LIDA_ERR, "socket");
      return ERROR;
   }
   
   bzero (&options, sizeof (struct sockaddr_in));
   
   options.sin_family = AF_INET;
   options.sin_port = htons (true_type.port);
   options.sin_addr.s_addr = htonl (INADDR_ANY);
   
   if (bind (socket_fd, (struct sockaddr *)&options, sizeof (struct sockaddr_in)) < 0)
   {
      lidalog (LIDA_ERR, "bind");
      return ERROR;
   }
   
   if (listen (socket_fd, true_type.connect * true_type.listener) < 0)
   {
      lidalog (LIDA_ERR, "listen");
      return ERROR;
   }
   
   return socket_fd;
}


void check_client (int sig)
{
   int status, chid, pid, i;
   pid_t m = getpid ();
   
   /*Check which one is dead*/
   while ((chid = waitpid (0, &status, WNOHANG)) >= 0)
   {
      if (WIFSIGNALED (status) && !QUIT_flag){
        /*cache proc end*/
        if (chid == cache_pid)
	{
	   lidalog (LIDA_ACCESS, "cache proc end up unproper, restarting");
           /*Restart cache process*/
           pid = fork ();
           if (pid < 0)
	      ERR_EXIT ("fork");
           else if (pid == 0)
	      lida_cache_proc (m);
	   else cache_pid = pid;
	}

	else
	{
	   for (i=0; i<true_type.listener; i++)
	   {
	      if (*(listener_lst+i) == chid)
	      {
	         lidalog (LIDA_ACCESS, "listenser process died unproper, restart");
		 /*Resstart listener process*/
		 pid = fork ();
		 if (pid < 0)
		    ERR_EXIT ("fork");
		 else if (pid == 0)
		    listener_proc (msock, m, cache_pid);
	      }
	   }
	}
      }
   }
}


void read_exit (int sig)
{
   int i;
   QUIT_flag = 1;
   
   /*Send all child processes SIGINT signal*/
   if (kill (0, SIGINT) < 0)
      lidalog (LIDA_ERR, "kill");

   lidalog (LIDA_ACCESS, "Exiting.........");

   for (i=0; i<true_type.listener; i++)
      waitpid (*(listener_lst+i), NULL, 0);
   lidalog (LIDA_ACCESS, "All listener is down");

   waitpid (cache_pid, NULL, 0);
   exit (0);
}



