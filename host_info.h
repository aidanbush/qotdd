/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 24, 17
 * File: host_info.h
 * Description: header file for host_info
 */

#ifndef HOST_INFO_H
#define HOST_INFO_H

typedef struct host_info {
    char *host;
    char *port;
    char *path;
    char *key;
} host_info;

host_info *parse_host_info(char *, char *);

void clean_host_info(host_info *);

#endif /* HOST_INFO_H */
