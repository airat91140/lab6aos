/**
 * @file daemon.h
 * @author Airat Makhmutov
 * @brief Functions related with launching of daemon
 * @version 0.1
 * @date 2023-12-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef D0B7CAB6_9F58_422C_BD9C_D185C641FA2F
#define D0B7CAB6_9F58_422C_BD9C_D185C641FA2F

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief sequence of actions to become daemon
 * 
 * @return int error code. If zero - its ok.
 */
int launch_daemon();

/**
 * @brief Function for closing all file descriptors
 * 
 */
void close_fds();

/**
 * @brief forks itself and goes to background
 * 
 * @return int error code. If zero - its ok.
 */
int run_bg();


#endif /* D0B7CAB6_9F58_422C_BD9C_D185C641FA2F */
