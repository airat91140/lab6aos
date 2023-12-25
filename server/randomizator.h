/**
 * @file randomizator.h
 * @author Airat Makhmutov)
 * @brief radomizator worker
 * @version 0.1
 * @date 2023-12-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef D6DF6870_133B_4587_927A_A79FF58F814A
#define D6DF6870_133B_4587_927A_A79FF58F814A

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

#include "server_utils.h"

/**
 * @brief randomizer worker
 * 
 * @param min_num mininmum bound for randomization
 * @param max_num maximum bound for randomization
 * @return int error code. Negative on error
 */
int randomization(int min_num, int max_num);

#endif /* D6DF6870_133B_4587_927A_A79FF58F814A */
