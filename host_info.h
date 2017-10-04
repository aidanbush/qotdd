/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 24, 17
 * File: host_info.h
 * Description: The header file for host_info.c, this file contains the
 *  structure for holding the information needed to create connections and, the
 *  JSON key.
 */

#ifndef HOST_INFO_H
#define HOST_INFO_H

/* structures meant to hold the arguments after they have been parsed */
typedef struct host_info {
    char *host;
    char *port;
    char *path;
    char *key;
} host_info;

/* The argument parsing function.
 * Input: path = The argument to be parsed.
 *        key = The key for JSON.
 * The function takes both non optional arguments, parses and combines them in
 *  then host_info structure, after allocating space and copying them over.
 *  (it does not allocate space for the key only points it since it is the only
 *  argument that does not have to be modified)
 * Return: Host_info structure containing the parsed arguments.
 */
host_info *parse_host_info(char *path, char *key);

/* The cleanup function for the host_info structure.
 * Input: info = The host_info structure to be destroyed.
 * The function takes a host info struct and frees the allocated space in it
 *  and then frees the struct itself.
 */
void clean_host_info(host_info *info);

#endif /* HOST_INFO_H */
