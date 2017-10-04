/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 24, 17
 * File: host_info.c
 * Description: The file used to break apart, and store the host, port, path,
 *  and JSON key, given.
 */

/* standard libraries */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* project includes */
#include "host_info.h"

#define DEFAULT_PORT "80"

extern int v;

// checks if a port is specified in the argument
int check_port(char *path) {
    for (int i = 0; i < strlen(path); i++)
        if (path[i] == ':')
            return 1;

    return 0;
}

// parses and returns a host_info struct pointer
host_info *parse_host_info(char *path, char *key) {
    host_info *info = malloc(sizeof(host_info));

    if (info == NULL) {
        if (v >= 1) perror("host info malloc");
        return NULL;
    }

    int has_port = check_port(path);

    char *str[3] = {NULL, NULL, NULL};

    if (has_port) {
        // grabs host, port, and path
        str[0] = strtok(path, ":");
        str[1] = strtok(NULL, "/");
        str[2] = strtok(NULL, "\0");
        // copies over port if it exists
        if (str[1] != NULL)
            info->port = strdup(str[1]);
        else
            info->port = NULL;
        // copies over path if it exists
        if (str[2] != NULL){
            info->path = malloc(sizeof(char) * (strlen(str[2]) + 2));
            if (info->path != NULL)
                sprintf(info->path, "/%s", str[2]);
        } else {
            info->path = NULL;
        }
    } else {
        //grabs host and path
        str[0] = strtok(path, "/");
        str[2] = strtok(NULL, "\0");

        info->port = NULL;
        // copies over path if it exists
        if (str[2] != NULL) {
            info->path = malloc(sizeof(char) * (strlen(str[2]) + 2));
            if (info->path != NULL)
                sprintf(info->path, "/%s",str[2]);
        } else {
            info->path = NULL;
        }
    }

    // set host
    info->host = strdup(str[0]);

    // if path is null, sets to root
    if (info->path == NULL) {
        info->path = strdup("/");
    }

    // if the port is NULL set to the default option
    if (info->port == NULL)
        info->port = strdup(DEFAULT_PORT);

    // set key
    info->key = key;

    // if any of the strdup calls failed
    if (info->path == NULL ||
        info->host == NULL ||
        info->port == NULL)
        return NULL;

    return info;
}

// frees the host info object and all its elements
void clean_host_info(host_info *info) {
    free(info->host);
    free(info->port);
    free(info->path);
    free(info);
}
