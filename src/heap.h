struct heap_t
{
   unsigned int key;
   void *data;
};

struct heap_h
{
   int size;
   int cnt;
   struct heap_t *p;
};

# define HEAP_FULL(X) ((X)->cnt == ((X)->size - 1))
# define HEAP_EMPTY(X) (!((X)->cnt))
