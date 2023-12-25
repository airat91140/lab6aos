#ifndef CCB6117B_FC6B_490D_A71C_EA11EDA896D1
#define CCB6117B_FC6B_490D_A71C_EA11EDA896D1

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "server_utils.h"

/**
 * @brief initializes server socket
 * 
 * @param sock_fd socket created by sock_fd function
 * @return int error code. Negative on error
 */
int init_connection(int sock_fd);

/**
 * @brief closes server socket
 * 
 * @return int error code. Negative on error
 */
int close_connection();


#endif /* CCB6117B_FC6B_490D_A71C_EA11EDA896D1 */
