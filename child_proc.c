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
#include <string.h>

/* system libraries */
#include <sys/types.h>
#include <sys/socket.h>

/* project includes */
#include "host_info.h"

#define TEST_SERVICE "80" // TODO: replace
#define TEST_HOST "date.jsontest.com"

#define SIZE_OF_GET_MESSAGE 26
#define RES_BUF_SIZE 1024

extern int v;

int make_client_socket(host_info_struct *info) {
    struct addrinfo *res, hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_V4MAPPED
    };

    if (v >= 1) fprintf(stdout, "creating client socket\n");

    int err = getaddrinfo(info->host, info->port, &hints, &res);
    if (err != 0) {
        if (v >= 1) fprintf(stderr, "%s\n", gai_strerror(err));
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

    if (cur == NULL) {
        fprintf(stdout, "cur == NULL\n");
        return -1;
    }

    if (v >= 1) fprintf(stdout, "client socket and connection created\n");

    return cfd;
}

//get request to server
int send_get_req(int qfd, host_info_struct *info) {
    char *msg = malloc(sizeof(char) *
            ((SIZE_OF_GET_MESSAGE) + strlen(info->path) + strlen(info->host)
             + strlen(info->port) + 1));

    // create message to be sent
    int err = sprintf(msg, "GET %s HTTP/1.1\r\n"
                "Host: %s:%s\r\n"
                "\r\n",
                info->path, info->host, info->port);

    if (err < 0) {
        if (v >= 1) fprintf(stderr, "unable to send message due to sprintf "
                                    "error\n");
        return -1;
    }

    int wrttn = send(qfd, msg, strlen(msg), 0);
    if (wrttn == -1) {
        if (v >= 3) fprintf(stderr, "client send error\n");
    } else {
        if (v >= 3) fprintf(stderr, "client send successful\n");
    }
    return 1;
}

int add_to_response(char *buf, char **res) {
    if (*res == NULL){
        *res = calloc(RES_BUF_SIZE, sizeof(char));
        if (*res == NULL) return -1;

        strncpy(*res, buf, strlen(buf));
    } else {
        int res_len = strlen(*res);
        char *temp_ptr = calloc(strlen(*res) + strlen(buf), sizeof(char));
        if (temp_ptr == NULL) return -1;

        strncpy(temp_ptr, *res, res_len); // copy over res
        strncpy(&temp_ptr[res_len], buf, strlen(buf));// copy over buf

        free(*res);// free res
        *res = temp_ptr;// set res
    }
    return 1;
}

// requests a quote from the server provied earlier
char* request_quote(host_info_struct *info) {
    char *quote = NULL;

    // make client socket
    int qfd; // quote file descriptor
    qfd = make_client_socket(info);

    if (qfd == -1) {
        if (v >= 1) fprintf(stderr, "failed to create socket or connect\n");
        exit(1);
    }

    // test response
    int err = send_get_req(qfd, info);

    if (err == -1)
        return NULL;

    int n;
    char buf[RES_BUF_SIZE];
    memset(&buf, 0, RES_BUF_SIZE);
    char *res = NULL;

    // fails on read does not stop (does not read nothing and stop???)
    while ((n = read(qfd, buf, sizeof(buf) - 1)) >= 0) {
        buf[n] = '\0';
        add_to_response(buf, &res);
        if (n < RES_BUF_SIZE - 1)
            break;
    }

    fprintf(stdout, "printing res\n");
    fprintf(stdout, "res:%s", res);

    if (n < 0)
        if (v >= 1) perror("read");

    // send request

    // parse quote

    close(qfd);

    // return quote
    return quote;
}

// main child process makes requests and responds to the connecting client
void child_proc(int cfd, host_info_struct *info) {
    char *quote;

    // make request to server
    quote = request_quote(info);

    if (quote == NULL) {
        if (V >= 1) fprintf(stderr, "unable to get quote\n");
        exit(1);
    }
    

    // parse

    // forward

    close(cfd);
    if (v >= 3) fprintf(stdout, "child closed connection cfd:%d\n", cfd);

    exit(0); // kill
}
