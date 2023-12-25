/**
 * @file logger.h
 * @author Airat Makhmutov (you@domain.com)
 * @brief functions for logging worker
 * @version 0.1
 * @date 2023-12-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef B4125C0E_F62E_4531_ACE2_D37C2D356A44
#define B4125C0E_F62E_4531_ACE2_D37C2D356A44

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "server_utils.h"

/**
 * @brief launches logger process
 * 
 * @return int error code. Negative on error
 */
int logger();

/**
 * @brief gets message from queue and passes it to log
 * 
 * @param msq_id id of message queue
 * @return int error code. Negative on error, 1 on success, zero on signal interrupt
 */
int logger_listen(int msq_id);

/**
 * @brief writes message into logfile from shared memory
 * 
 * @param msg the message to write
 * @return int error code. Negative on error
 */
int write2log(logger_msg msg);

#endif /* B4125C0E_F62E_4531_ACE2_D37C2D356A44 */
