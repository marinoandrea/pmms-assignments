#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "input.h"

int parse_arguments(int i_argc, char **i_argv, params* o_params)
{
    int c; 
    
    while((c = getopt(i_argc, i_argv, "adrgp:l:s:")) != -1) {
        switch(c) {
            case 'a':
                o_params->order = ASCENDING;
                break;
            case 'd':
                o_params->order = DESCENDING;
                break;
            case 'r':
                o_params->order = RANDOM;
                break;
            case 'l':
                o_params->length = atol(optarg);
                break;
            case 'g':
                o_params->debug = 1;
                break;
            case 's':
                o_params->seed = atoi(optarg);
                break;
            case 'p':
                o_params->num_threads = atoi(optarg);
                break;
            case '?':
                if(optopt == 'l' || optopt == 's') 
                {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if(isprint(optopt)) 
                {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                }
                else 
                {
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                }
                return -1;
            default:
                return -1;
        }
    }

    return 0;
}