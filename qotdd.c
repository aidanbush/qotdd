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
#include <netdb.h>
#include <errno.h>

/* system libraries */
#include <sys/types.h>
#include <sys/socket.h>

/* project includes */
#include "child_proc.h"

#define EXIT_INVALID_OPT 2
#define EXIT_SOCKET_FAIL 3

#define SOCKET_RETRY 5
#define PORT "1042"
#define BACKLOG 5

int v;

void print_usage(char *p_name) {
    printf("usage : %s [-options] host[:port]/path key\n"
           "Implements quote of the day by forwarding the quote it retrieves\n"
           " from the given server.\n\n"
           "Options:\n"
           "    -h  Displays this help message and exit\n"
           "    -v  Verbose [use more than once more for move verbose]\n"
           "        v give basic debuging including errors\n"
           "        vv adds logging of incoming and outgoing messages\n"
           "        vvv adds logging of all traffic\n\n"
           "required options:\n"
           "  host  The destination of the server to forward from\n"
           "  port  The specific port to recieve the quote from\n"
           "  path  The path to the quote\n"
           "  key   The corresponding JSON key for the quote\n", p_name);
}

int server_proc(char *path, char *key) {
    struct addrinfo *res, hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE | AI_V4MAPPED
        // add TCP protocol
    };

    int err;
    int sfd;

    /* This Code was taken from the example in lab 1 and modified */
    // for socket retry times
    for (int i = 0; i < SOCKET_RETRY; i++) {
        err = getaddrinfo(NULL, PORT, &hints, &res);
        if (err != 0)
            continue;
        struct addrinfo *cur;
        // for all sockets from addrinfo
        for (cur = res; cur != NULL; cur = cur->ai_next) {
            // create socket
            sfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);

            if (sfd == -1) {
                if (v >= 1) perror("socket");
                continue;
            }

            // socket options
            int val = 1;
            err = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
            if (err == -1) {
                if (v >= 1) perror("setsockopt");
                close(sfd);
                continue;
            }

            // bind
            err = bind(sfd, cur->ai_addr, cur->ai_addrlen);
            if (err == -1) {
                if (v >= 1) perror("bind");
                close(sfd);
                continue;
            }

            // listen
            err = listen(sfd, BACKLOG);
            if (err == -1) {
                if (v >= 1) perror("listen");
                close(sfd);
                continue;
            }
            break;
        }

        // free addinfo
        freeaddrinfo(res);

        if (cur != NULL) break;
    }

    // setup signal interupt handler

    /* This code was taken from the lab 1 example and modified */
    // accept loop
    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int cfd = accept(sfd, (struct sockaddr*) &client_addr,
            &client_addr_len);
        if (cfd == -1) {
            // if the error was not caused by an interupt
            if (errno != EINTR)
                if (v >= 1) perror("accept");
            continue;
        }
        // fork here

        close(cfd);
    }

    return 0;
}

int main(int argc, char **argv) {
    char *path, *key;
    char c;
    v = 0;

    // getopt
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

    // get required fields
    if (optind == argc -2) {
        path = argv[optind];
        key = argv[optind + 1];
    } else {
        print_usage(argv[0]);
        exit(EXIT_INVALID_OPT);
    }

    return server_proc(path, key);
}
