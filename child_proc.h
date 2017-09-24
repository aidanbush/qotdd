/* Author: Aidan Bush
 * Assign: Assign 1
 * Course: CMPT 361
 * Date: Sept. 19, 16
 * File: child_proc.h
 * Description: the header file for the child processes
 */

#ifndef CHILD_PROC_H
#define CHILD_PROC_H

/* The main child proccess function
 * Input: cfd = file descriptor for the connection
 * The function takes the given connection and responds with the quote of the
 *  day
*/
void child_proc(int);

#endif /* CHILD_PROC_H */
