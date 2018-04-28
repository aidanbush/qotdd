/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 19, 17
 * File: child_proc.c
 * Description: This file handles the child process, both requesting they quote,
 *  parsing the output and responding to the original request.
 */

/* standard libraries */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

/* system libraries */
#include <sys/types.h>
#include <sys/socket.h>

/* project includes */
#include "host_info.h"
#include "jsmn/jsmn.h"

#define RES_BUF_SIZE 1024

extern int v;

/* creates the child socket that connects to the quote server and returns the
 * file descriptor
 */
int make_client_socket(host_info *info) {
    struct addrinfo *res, hints = {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_V4MAPPED
    };

    int err = getaddrinfo(info->host, info->port, &hints, &res);
    if (err != 0) {
        if (v >= 1) fprintf(stderr, "%s\n", gai_strerror(err));
        return -1;
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
        if (v >= 1) fprintf(stderr, "failed to connect to server\n");
        return -1;
    }

    return cfd;
}

// create and send get request to server
int send_get_req(int qfd, host_info *info) {
    // create message to be sent
    int msg_len = snprintf(NULL, 0, "GET %s HTTP/1.1\r\n"
                "Host: %s:%s\r\n"
                "Connection: close\r\n"
                "\r\n",
                info->path, info->host, info->port);

    char *msg = malloc(sizeof(char) * msg_len + 1);
    if (msg == NULL) {
        if (v >= 1) perror("malloc");
        return -1;
    }

    int err = sprintf(msg, "GET %s HTTP/1.1\r\n"
                "Host: %s:%s\r\n"
                "Connection: close\r\n"
                "\r\n",
                info->path, info->host, info->port);

    if (err < 0) {
        if (v >= 1) fprintf(stderr, "unable to send message due to sprintf "
                                    "error\n");
        free(msg);
        return -1;
    }

    if (v >= 3) fprintf(stdout, "\nsend GET message: %s\n", msg);

    if (v >= 2) fprintf(stdout, "sending request message\n");
    int wrttn = send(qfd, msg, strlen(msg), 0);
    if (wrttn == -1) {
        if (v >= 3) fprintf(stderr, "client send error\n");
    } else {
        if (v >= 3) fprintf(stderr, "client send successful\n");
    }
    free(msg);
    return 1;
}

// finds the start of the response message
char *body_start(char *res) {
    for (int i = 0; i < strlen(res); i++) {
        if (res[i] == '\r')
            if (strncmp(&res[i], "\r\n\r\n", 4) == 0)
                return &res[i+4];
    }
    return NULL;
}

// strcmp implemented to compare jsmn strings
int json_strcmp(char *json, jsmntok_t *tok, char *key) {
    if (tok->type == JSMN_STRING && strlen(key) == tok->end - tok->start)
        return strncmp(json + tok->start, key, tok->end - tok->start);
    return -1;
}

// calls jsmn_parse to calculate the number of tokens in the json object
int jsmn_num_toks(char *json) {
    jsmn_parser p;
    jsmn_init(&p);
    // currently returning -3 for string to short expected more
    return jsmn_parse(&p, json, strlen(json), NULL, 0);
}

// parses the quote out the quote from the message body
char *parse_quote(char *res, char *key) {
    char *start = body_start(res);

    int num_toks = jsmn_num_toks(start);
    if (num_toks < 1) {
        if (v >= 1) fprintf(stderr, "jsmn counted negative tokens %d\n",
                            num_toks);
        return NULL;
    }

    int c;
    jsmn_parser p;
    jsmntok_t t[num_toks];
    jsmn_init(&p);

    c = jsmn_parse(&p, start, strlen(start), t, num_toks);

    if (c < 0)
        return NULL;

    for (int i = 1; i < c; i++) {
        if (json_strcmp(start, &t[i], key) == 0) {
            return strndup(start + t[i+1].start, t[i+1].end - t[i+1].start);
        }
    }
    if (v >= 1) fprintf(stderr, "jsmn could not match keyword\n");
    return NULL;
}

// adds the given string to the end of the res string
int add_to_response(char *buf, char **res) {
    if (*res == NULL){
        *res = calloc(RES_BUF_SIZE, sizeof(char));
        if (*res == NULL) return -1;

        strncpy(*res, buf, strlen(buf));
    } else {
        int res_len = strlen(*res);
        int buf_len = strlen(buf);

        char *temp_ptr = calloc(res_len + buf_len + 1, sizeof(char));
        if (temp_ptr == NULL) return -1;

        strncpy(temp_ptr, *res, res_len); // copy over res
        strncpy(temp_ptr + res_len, buf, buf_len); // copy buf after res part
        temp_ptr[res_len + buf_len] = '\0'; // set null terminator

        free(*res); // free res
        *res = temp_ptr; // set res
    }
    return 1;
}

// creates and returns the error message for the given http code
char *create_err_msg(int code) {
    if (v >= 1) fprintf(stderr, "error response code: %d\n", code);

    int msg_len = snprintf(NULL, 0, "cannot obtain quote: %d", code);

    char *msg = malloc(sizeof(char) * msg_len + 1);
    if (msg == NULL) {
        if (v >= 1) perror("malloc");
        return NULL;
    }

    int err = sprintf(msg, "cannot obtain quote: %d", code);
    if (err < 0) {
        if (v >= 1) perror("sprintf");
        free(msg);
        return NULL;
    }
    return msg;
}

// checks the response code and creates an error message if it was not 2XX
int check_res_code(char *res, char **quote) {
    int j, i = 0;
    // get to beginning of code
    for (; i < strlen(res); i++)
        if (res[i] == ' ') break;
    i++;
    j = i;
    // get end space
    for (; j < strlen(res); j++)
        if (res[j] == ' ') break;

    char code_copy[4];

    // check if number else return -1 for error to be dealt with later
    for (int k = i; k < strlen(res) && k - i < 3; k++) {
        code_copy[k - i] = res[k];
        if (isdigit(res[k]) == 0) return -1;
    }
    code_copy[3] = '\0';

    // convert
    int code = atoi(code_copy);

    // if success
    if (code >= 200 && code < 300)
        return 1;

    // else was not successful
    *quote = create_err_msg(code);
    if (quote == NULL)
        return -1;
    return 1;
}

// requests a quote from the server provided earlier
char *request_quote(host_info *info) {
    char *quote = NULL;

    // make client socket
    int qfd; // quote file descriptor
    qfd = make_client_socket(info);

    if (qfd == -1) {
        if (v >= 1) fprintf(stderr, "failed to create socket or connect\n");
        exit(1);
    }

    // send get request
    int err = send_get_req(qfd, info);
    if (err == -1) {
        close(qfd);
        return NULL;
    }

    int n;
    char buf[RES_BUF_SIZE];
    char *res = NULL;
    // will get stuck if the server does not close connection
    while ((n = read(qfd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        add_to_response(buf, &res);
    }

    if (n < 0)
        if (v >= 1) perror("read");

    if (v >= 3) fprintf(stdout, "\nresponse message:%s", res);

    if (res == NULL) {
        if (v >= 1) fprintf(stderr, "unable to get response\n");
        return NULL;
    }

    // check res code
    err = check_res_code(res, &quote);

    // parse quote if an error msg has not been made
    if (quote == NULL && err != -1)
        quote = parse_quote(res, info->key);

    close(qfd);

    free(res);

    // return quote
    return quote;
}

// writes the quote message
void write_quote(char *quote, int cfd) {
    if (v >= 2) fprintf(stdout, "sending quote message\n");
    int wrttn = send(cfd, quote, strlen(quote), 0);

    if (wrttn != strlen(quote))
        if (v >= 1) fprintf(stderr, "quote send error\n");
}

// main child process makes requests and responds to the connecting client
void child_proc(int cfd, host_info *info) {
    char *quote;

    // make request to server and parse the quote
    quote = request_quote(info);

    if (quote == NULL) {
        if (v >= 1) fprintf(stderr, "unable to get quote, and an error code "
                                    "was not sent\n");
        close(cfd);
        exit(1);
    }

    // forward quote
    write_quote(quote, cfd);

    free(quote);

    close(cfd);
    if (v >= 2) fprintf(stdout, "child closed connection cfd: %d\n", cfd);

    exit(0); // kill
}
