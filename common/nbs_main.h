#define NBS_PARAM_TYPE_NONE 0
#define NBS_PARAM_TYPE_INT 1
#define NBS_PARAM_TYPE_BOOL 2
#define NBS_PARAM_TYPE_STR 3

#ifdef _OPENMP
# include <omp.h>
#else
# define omp_get_max_threads()  1
# define omp_get_thread_num()   0
# define omp_set_num_threads(x)
#endif

void nbs_print_usage(void);
void nbs_print_usage_option(char opt, int type, char* description, char *val, int subc, char **subv);
