typedef enum Ordering 
{
    ASCENDING, 
    DESCENDING, 
    RANDOM
} Order;

typedef struct 
{
    int seed;
    long length;
    int num_threads;
    Order order;
    int debug;
} params;

int parse_arguments(int i_argc, char **i_argv, params* o_params);