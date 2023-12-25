/**
 * @file round_worker.h
 * @author Airat Makhmutov (you@domain.com)
 * @brief file with worker for round processing
 * @version 0.1
 * @date 2023-12-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ACF5995D_AF95_4CCD_84C3_F9125CB85F4C
#define ACF5995D_AF95_4CCD_84C3_F9125CB85F4C

#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>

#include "server_utils.h"

/**
 * @brief main fucntion for processing one round
 * 
 * @return int error code. Negative on error
 */
int round_worker();

/**
 * @brief comparesuser value with generated and sends respected message
 * 
 * @param rnd_msg message with necessery information from master
 * @return int error code. Negative on error
 */
int check_cmp(round_worker_msg rnd_msg);

/**
 * @brief when game ends send results to clients
 * 
 * @return int error code. Negative on error
 */
int send_results();

#endif /* ACF5995D_AF95_4CCD_84C3_F9125CB85F4C */
