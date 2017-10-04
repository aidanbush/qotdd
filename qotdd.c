/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 19, 17
 * File: qotdd.c
 * Description: The main project file
 */

/* standard libraries */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <libgen.h>

/* system libraries */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

/* project includes */
#include "child_proc.h"
#include "host_info.h"

// exit fail defines
#define EXIT_INVALID_OPT 1
#define EXIT_SIGINT_FAIL 2
#define EXIT_SIGCHLD_FAIL 4
#define EXIT_SOCK_SETUP_FAIL 8
#define EXIT_OTHER_FAIL 16

#define PORT "1700"
#define BACKLOG 5

// the verbosity flag
int v = 0;
// the exit server flag
static bool exit_server = false;

// the function called for the sigint handler stops the main server loop
void sigint_handler(int par) {
    exit_server = true;
}

// creates the SIGINT interrupt handler
int create_sigint_handler() {
    struct sigaction interrupt = {
            .sa_handler = &sigint_handler
    };

    int err = sigaction(SIGINT, &interrupt, NULL);
    if (err == -1) {
        if (v >= 1) perror("sigaction");
        return -1;
    }
    return 0;
}

// the SIGCHLD handler
void sigchld_handler(int par) {
    waitpid(-1, NULL, WNOHANG);
}

// creates the SIGCHLD interrupt handler
int create_sigchld_handler() {
    struct sigaction interrupt = {
        .sa_handler = &sigchld_handler
    };

    int err = sigaction(SIGCHLD, &interrupt, NULL);
    if (err == -1) {
        if (v >= 1) perror("sigaction");
        return -1;
    }
    return 0;
}

// usage printing function takes argv[0] and prints the usage message
void print_usage(char *p_name) {
    printf("usage : %s [-options] host[:port]/path key\n"
           "Implements quote of the day by forwarding the quote it retrieves\n"
           " from the given server.\n\n"
           "Options:\n"
           "    -h  Displays this help message and exit\n"
           "    -v  Verbose [use more than once more for move verbose]\n"
           "        v give basic debugging of errors\n"
           "        vv adds logging of  messages and important events\n"
           "        vvv adds logging of all traffic\n\n"
           "required options:\n"
           "  host  The destination of the server to forward from\n"
           "  port  The specific port, uses 80 if not included\n"
           "  path  The path to the quote\n"
           "  key   The corresponding JSON key for the quote\n",
            basename(p_name));
}

// creates the server socket and returns its file descriptor on error returns -1
int make_server_socket(host_info *info) {
    // TODO: test that works with ipv4 and ipv6
    struct addrinfo *res, hints = {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE | AI_V4MAPPED
    };

    int err;
    int sfd;

    /* This Code was taken from the example in lab 1 and modified */
    err = getaddrinfo(NULL, PORT, &hints, &res);
    if (err != 0) {
        if (v >= 1) fprintf(stderr, "gai_err %s\n", gai_strerror(err));
        return -1;
    }
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

    if (cur == NULL) {
        if (v >= 1) fprintf(stderr, "failed to connect\n");
        close(sfd);
        return -1;
    }
    return sfd;
}

// the main server setup and loop function
int server_proc(host_info *info) {
    int sfd, err;

    sfd = make_server_socket(info);
    if (sfd == -1) {
        return EXIT_SOCK_SETUP_FAIL;
    }

    // setup signal interrupt handler
    err = create_sigint_handler();
    if (err == -1) {
        close(sfd);
        return(EXIT_SIGINT_FAIL);
    }

    err = create_sigchld_handler();
    if (err == -1) {
        close(sfd);
        return(EXIT_SIGCHLD_FAIL);
    }

    /* This code was taken from the lab 1 example and modified */
    // accept loop
    while (!exit_server) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int cfd = accept(sfd, (struct sockaddr*) &client_addr,
            &client_addr_len);
        if (cfd == -1) {
            // if the error was not caused by an interrupt
            if (errno != EINTR)
                if (v >= 1) perror("accept");
            continue;
        }
        if (v >= 2) fprintf(stdout, "opened connection cfd: %d\n", cfd);

        int pid = fork();
        // if child
        if (pid == 0) {
            child_proc(cfd, info);
        }

        close(cfd);
        if (v >= 2) fprintf(stdout, "parent closed connection cfd: %d\n", cfd);
    }

    close(sfd);

    return 0;
}

// main function gets options and then calls the server process function
int main(int argc, char **argv) {
    char *path, *key;
    char c;

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

    host_info * info = parse_host_info(path, key);

    if (info == NULL) {
        print_usage(argv[0]);
        exit(EXIT_OTHER_FAIL);
    }

    int serv_exit = server_proc(info);

    clean_host_info(info);

    return serv_exit;
}
