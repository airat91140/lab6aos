/**
 * @file master.h
 * @author Airat Makhmutov (you@domain.com)
 * @brief master process fucntions
 * @version 0.1
 * @date 2023-12-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef CDEECB6B_021B_406D_9268_6EF368CF052E
#define CDEECB6B_021B_406D_9268_6EF368CF052E

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "configger.h"
#include "server_utils.h"
#include "logger.h"
#include "tcp_worker.h"
#include "randomizator.h"
#include "client_listener.h"
#include "round_worker.h"

int init();

int master();

/**
 * @brief collecting tcp messages and manages the bigining of game
 * 
 * @return int error code. Negative on error
 */
int listen_requests(int sock_fd);

/**
 * @brief destructor for server
 * 
 * @return int error code. Negative on errror
 */
int die();

/**
 * @brief checks correctness of game request, adds requests to shared memory, removes them from it
 * 
 * @param client request for the game from tcp
 * @param hostfd returns host fd when we can start the game, -1 when we cant start game yet
 * @return int error code. Negative on error
 */
int check_game(tcp_worker_msg client, int *hostfd);

/**
 * @brief first phase when server inits values needed for a game
 * 
 * @param client1 fd of first client
 * @param client2 fd of second client
 * @return int error code. Negative on error
 */
int start_game(int client1, int client2);

/**
 * @brief function for shootout phase of game
 * 
 * @param fd1 socket descriptor of first client
 * @param fd2 socket descriptor of second client
 * @return int error code. Negative on error
 */
int shootout(int fd1, int fd2);

/**
 * @brief processing data from listener worker
 * 
 * @return int error code. Negative on error
 */
int listener_processing();

int send_hints(shared_mem *mem);

void sigterm_handler(int signum);

#endif /* CDEECB6B_021B_406D_9268_6EF368CF052E */
