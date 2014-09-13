# include "manager.h"
# include "timer.h"

pthread_t manager_id;


void timer_trigger (int signo)
{
   timer_task (listener_timer); 
}


void manager_thread (void *arg)
{
   int err;

   /*Set up manager signal functions*/
   struct sigset_t sigset;

   sigemptyset (&sigset);
   sigaddset (&sigset, SIGUSR1);
   sigaddset (&sigset, SIGUSR2);
   sigaddset (&sigset, SIGINT);
   sigaddset (&sigset, SIGCHLD);

   if (err = pthread_sigmask (SIG_BLOCK, &sigset, NULL)){
      lidalog (LIDA_ACCESS, strerror (err));
      pthread_exit ((void *)-1);
   }

   /*Init timer*/
   listener_timer = timer_init ();

   /*Delete timeout events*/
   struct sigaction act;

   act.sa_handler = timer_trigger;

   if (sigaction (SIGALRM, &act, NULL) < 0){
      lidalog (LIDA_ERR, "sigaction");
      pthread_exit ((void *)-1);
   }

   /*Active message queue*/
   int msg_id = msgget (ftok ("/var/log/lida", 0), IPC_EXCL);

   if (msg_id < 0){
      lidalog (LIDA_ERR, "msgget");
      pthread_exit ((void *)-1);
   }

   /*Start timer*/
   time_t sa_time = true_type.keep_alive;
   alarm (sa_time?sa_time:1);

   /*Waiting for commands*/
   size_t m_size;
   struct master_cmd m_cmd;

   while (1){
      m_size = msgrcv (msg_id, (void *)&m_cmd, sizeof (struct master_cmd), 0);

      if (m_size < 0){
         lidalog (LIDA_ERR, "msgrcv");
	 raise (SIGCHLD);
	 pthread_exit ((void *)-1);
      }

      if (m_size < sizeof (struct master_cmd)){
         lidalog (LIDA_ACCESS, "Failed to receive master command");
	 continue;
      }

      switch (m_cmd.type)
      {
         case UPDATE_LOG: update_log (&m_cmd); break;
	 case ADD_PROXY: add_proxy (&m_cmd); break;
	 case DEL_PROXY: del_proxy (&m_cmd); break;
	 default: continue;
      }
   }
}


void update_log (struct master_cmd *cmd)
{
   
}


void add_proxy (struct master_cmd *cmd)
{
   
}


void del_proxy (struct master_cmd *cmd)
{
   
}
