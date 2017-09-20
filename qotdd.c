/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 19, 17
 * File: qotdd.c
 * Description: the main project file
 */

/* standard libraries */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* system libraries */

/* project includes */
#include "child_proc.h"

#define EXIT_INVALID_OPT 2

int v;

void print_usage(char *p_name) {
    printf("usage : %s [-options] host[:port]/path key\n"
           "Implements quote of the day by forwarding the quote it retrieves\n"
           " from the given server.\n\n"
           "Options:\n"
           "    -h  Displays this help message and exits\n"
           "    -v  Verbose [use more than once more for move verbose]\n\n"
           "required options:\n"
           "  host  The destination of the server to forward from\n"
           "  port  The specific port to recieve the quote from\n"
           "  path  The path to the quote\n"
           "  key   The corresponding JSON key for the quote\n", p_name);
}

int main(int argc, char **argv) {
    char *path, *key;
    char c;
    v = 0;

    //getopt
    while ((c = getopt(argc, argv, "vh")) != -1) {
        switch (c) {
            case 'v':
                v++;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            default:
                print_usage(argv[0]);
                exit((EXIT_INVALID_OPT));
        }
    }

    //get required fields
    if (optind == argc -2) {
        path = argv[optind];
        key = argv[optind + 1];
    } else {
        print_usage(argv[0]);
        exit(EXIT_INVALID_OPT);
    }

    return 0;
}
