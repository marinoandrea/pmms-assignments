#include <stdlib.h>
#include <stdatomic.h>

void histogram(atomic_int * histo, int * image, int n_threads, size_t img_size);
