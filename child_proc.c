/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 19, 17
 * File: child_proc.c
 * Description: This file handles the child process
 */

/* standard libraries */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

/* system libraries */
#include <sys/types.h>
#include <sys/socket.h>

/* project includes */

#define TEST_SERVICE "17" // TODO: replace
#define TEST_HOST "cygnus-x.net"

extern int v;

int make_client_socket() {
    struct addrinfo *res, hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_V4MAPPED
    };

    int err = getaddrinfo(NULL, TEST_SERVICE, &hints, &res);
    if (err != 0) {
        if (v >= 1) fprintf(stderr, "gai_strerror: %s\n", gai_strerror(err));
        exit(1);
    }

    // create socket
    int cfd;
    struct addrinfo * cur;
    for (cur = res; cur != NULL; cur = cur->ai_next) {
        // socket
        cfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (cfd < 0) {
            if (v >= 1) perror("socket");
            continue;
        }

        // connect
        err = connect(cfd, cur->ai_addr, cur->ai_addrlen);
        if (err == -1) {
            if (v >= 1) perror("connect");
            close(cfd);
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (cur == NULL)
        cfd = -1;

    return cfd;
}

// requests a quote from the server provied earlier
void request_quote() {
    // make client socket
    int qfd; // quote file descriptor
    qfd = make_client_socket();

    if (qfd == -1) {
        if (v >= 1) fprintf(stderr, "failed to create socket or connect\n");
        exit(1);
    }

    // test response
    int n;
    char buf[1024];
    while ((n = read(qfd, buf, sizeof(buf) - 1)) > 0) {
        printf("%s", buf);
    }

    if (n < 0) {
        perror("read");
    }

    // send request

    // parse quote

    close(qfd);

    // return quote
}

// main child process makes requests and responds to the connecting client
void child_proc(int cfd) {
    // make request to server
    request_quote();

    // parse

    // forward

    close(cfd);
    if (v >= 3) fprintf(stdout, "closed connection cfd:%d\n", cfd);

    exit(0); // kill
}
