/**
 * @brief Represents different strategies for generating arrays of integers.
 */
typedef enum 
{
    /* Numbers are sorted in ascending order. */
    ASCENDING,
    /* Numbers are sorted in descending order. */
    DESCENDING, 
    /* Numbers are not sorted. */
    RANDOM
} order_strategy;

/**
 * @brief Represents CLI arguments received when running the program.
 */
typedef struct 
{
    /* Numerical seed to initialize the random generation. Allows for reproducible experiments. */
    int seed;
    /* Length of the array to generate. */
    long length;
    /* Number of threads that the program should use. */
    int num_threads;
    /* Number of threads that the program should use. */
    order_strategy order;
    /* Whether the program should print debug information (if x >= 1 then print). */
    int debug;
} params;


/**
 * @brief Parse the CLI arguments and stores them in an ad-hoc struct.
 * Returns 1 if arguments are invalid else 0.
 * 
 * @param i_argc The count of received arguments.
 * @param i_argv The array of arguments strings.
 * @param o_params The structure to store data into.
 * @return int 
 */
int parse_arguments(int i_argc, char **i_argv, params* o_params);