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
#include "jsmn/jsmn.h"

#define SIZE_OF_GET_MESSAGE 26
#define RES_BUF_SIZE 1024

#define NUM_TOKS 128

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
                // TODO properly build "HOST" ":" host [ ":" port]
                "Host: %s:%s\r\n"
                "\r\n",
                info->path, info->host, info->port);

    if (v >= 3) fprintf(stdout, "send msg: |%s|\n", msg);

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

char *body_start(char *res) {
    for (int i = 0; i < strlen(res); i++) {
        if (res[i] == '\r')
            if (strncmp(&res[i], "\r\n\r\n", 4) == 0)
                return &res[i+4];
    }
    return NULL;
}

int json_strcmp(char *json, jsmntok_t *tok, char *key) { 
    if (tok->type == JSMN_STRING && strlen(key) == tok->end - tok->start)
        return strncmp(json + tok->start, key, tok->end - tok->start);
    return -1;
} 

char *parse_quote(char *res, char *key) {
    char *start = body_start(res);

    int c;
    jsmn_parser p;
    jsmntok_t t[NUM_TOKS];

    jsmn_init(&p);

    c = jsmn_parse(&p, start, strlen(start), t, NUM_TOKS);

    if (c < 0)
        return NULL;

    for (int i = 1; i < c; i++) {
        if (json_strcmp(start, &t[i], key) == 0) {
            return strndup(start + t[i+1].start, t[i+1].end - t[i+1].start);
        }
    }
    return NULL;
}

int add_to_response(char *buf, char **res) {
    if (*res == NULL){
        *res = calloc(RES_BUF_SIZE, sizeof(char));
        if (*res == NULL) return -1;

        strncpy(*res, buf, strlen(buf));
    } else {
        int res_len = strlen(*res);
        int buf_len = strlen(buf);

        char *temp_ptr = calloc(strlen(*res) + buf_len + 1, sizeof(char));
        if (temp_ptr == NULL) return -1;

        strncpy(temp_ptr, *res, res_len); // copy over res
        strncpy(&temp_ptr[res_len], buf, buf_len);// copy over buf
        temp_ptr[res_len + buf_len] = '\0';// set null terminator

        free(*res); // free res
        *res = temp_ptr; // set res
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
    if (err == -1) {
        close(qfd);
        return NULL;
    }

    int n;
    char buf[RES_BUF_SIZE];
    char *res = NULL;
    while ((n = read(qfd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        add_to_response(buf, &res);
        if (n < RES_BUF_SIZE - 1)
            break;
    }

    if (n < 0)
        if (v >= 1) perror("read");

    if (res == NULL) {
        if (v >= 1) fprintf(stderr, "unable to get response\n");
        return NULL;
    }

    // parse quote
    quote = parse_quote(res, info->key);

    close(qfd);

    // return quote
    return quote;
}

void write_quote(char *quote, int cfd) {
    int wrttn = send(cfd, quote, strlen(quote), 0);

    if (wrttn != strlen(quote))
        if (v >= 1) fprintf(stderr, "quote send error\n");
}

// main child process makes requests and responds to the connecting client
void child_proc(int cfd, host_info_struct *info) {
    char *quote;

    // make request to server and parse the quote
    quote = request_quote(info);

    if (quote == NULL) {
        if (v >= 1) fprintf(stderr, "unable to get quote\n");
        close(cfd);
        exit(1);
    }
    
    // forward
    write_quote(quote, cfd);

    free(quote);

    close(cfd);
    if (v >= 3) fprintf(stdout, "child closed connection cfd:%d\n", cfd);

    exit(0); // kill
}
