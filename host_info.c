/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 24, 17
 * File: host_info.c
 * Description: file used to break appart, and store the host, port, path, and
 *  key, given.
 */

/* standard libraries */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* system libraries */

/* project includes */
#include "host_info.h"

extern int v;

int check_port(char *path) {
    for (int i = 0; i < strlen(path); i++)
        if (path[i] == ':')
            return 1;

    return 0;
}

// parses and retuns a host_info struct pointer
host_info_struct *parse_host_info(char *path, char *key) {
    host_info_struct *host_info = malloc(sizeof(host_info_struct));

    if (host_info == NULL) {
        if (v >= 1) perror("host info malloc");
        return NULL;
    }

    int has_port = check_port(path);

    char *str[3] = {NULL, NULL, NULL};

    str[0] = strtok(path, ":/");
    for (int i = 1; i < 3; i++)
        str[i] = strtok(NULL, ":/");

    // set host
    host_info->host = strdup(str[0]);

    // check if port exists and set the port and path
    if (has_port) {
        if (str[1] != NULL)
            host_info->port = strdup(str[1]);
        else
            host_info->port = NULL;
        if (str[2] != NULL)
            host_info->path = strdup(str[2]);
        else
            host_info->path = NULL;
    } else {
        host_info->port = NULL;
        if (str[1] != NULL)
            host_info->path = strdup(str[1]);
        else
            host_info->path = NULL;
    }

    // if path is null, sets to root
    if (host_info->path == NULL) {
        host_info->path = strdup("/");
    }

    // if if port in NULL set to 17
    if (host_info->port == NULL)
        host_info->port = strdup("17");

    // set key
    host_info->key = key;

    // if any of the strdup calls failed
    if (host_info->path == NULL ||
        host_info->host == NULL ||
        host_info->port == NULL)
        return NULL;

    return host_info;
}

void clean_host_info(host_info_struct *host_info) {
    free(host_info->host);
    free(host_info->port);
    free(host_info->path);
    free(host_info);
}