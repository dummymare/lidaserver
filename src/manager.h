struct neo_proxy_info 
{
   char group_id[15];
   char ip[15];
   unsigned short port;
   int weight;
};

union master_cmd_msg
{
   char log_path[50];
   struct neo_proxy_info n_proxy;
};

struct master_cmd
{
   long type;
   union master_cmd_msg data;
};

# define UPDATE_LOG 0
# define ADD_PROXY 1
# define DEL_PROXY 2 
