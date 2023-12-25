/**
 * @file client_listener.h
 * @author Airat Makhmutov (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef FDC49E6D_D07A_4C0C_97C1_0301ECEAF645
#define FDC49E6D_D07A_4C0C_97C1_0301ECEAF645

#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>

#include "server_utils.h"

/**
 * @brief listens client activity
 * 
 * @param cl_id id of client. 0 or 1
 * @param fd socket descriptor of client
 * @return int error code. Negative on error
 */
int client_listener(int cl_id, int fd);

#endif /* FDC49E6D_D07A_4C0C_97C1_0301ECEAF645 */
