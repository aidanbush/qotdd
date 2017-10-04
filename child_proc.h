/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 19, 16
 * File: child_proc.h
 * Description: The header file for the child_proc.c, this file contains the
 *  function that runs the child process. It creates the connection to the
 *  given server and responds with the quotes after it is parsed using the JSON
 *  key provided. If the key is not found or the connection to get the quote
 *  cannot be established the connection will be closed. If there was an error
 *  code in the response that error code is forwarded.
 */

#ifndef CHILD_PROC_H
#define CHILD_PROC_H

#include "host_info.h"

/* The main child process function
 * Input: cfd = File descriptor for the connection.
 *        info = The parsed arguments structure used to create the connection
 *                and provides the key for parsing the json response.
 * The function takes the given connection and responds with the quote of the
 *  day
*/
void child_proc(int cfd, host_info *info);

#endif /* CHILD_PROC_H */
