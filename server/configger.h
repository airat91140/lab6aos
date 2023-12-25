/**
 * @file configger.h
 * @author Airat Makhmutov (you@domain.com)
 * @brief config updating worker
 * @version 0.1
 * @date 2023-12-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef C1F8614C_E3C1_40B7_8844_27CCDC5EE6C0
#define C1F8614C_E3C1_40B7_8844_27CCDC5EE6C0

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
#include <time.h>
#include <stdlib.h>

#include "server_utils.h"

#define CONFBUF_SZ 4096
#define LINEBUF_SZ 256

const static char *config_filename = "/home/airat/.lab6aos";

/**
 * @brief calls check of config file and updates memory
 * 
 * @return int negative value on error
 */
int config_update();

/**
 * @brief parses config file and passes it to the memory
 * 
 * @param mem shared memory with program data
 * @param fd descriptor of config file
 * @return int negativ value on error
 */
int parse_config(shared_mem *mem, int fd);

/**
 * @brief reads one line in given fd
 * 
 * @param line_buf buf to put written line
 * @param fd file to read
 * @return int error code or number of readen charecters. -1 if line is larger than LINEBUF_SZ, 
 * -2 if error during read, 0 if EOF
 */
int read_line(char line_buf[LINEBUF_SZ], int fd);

#endif /* C1F8614C_E3C1_40B7_8844_27CCDC5EE6C0 */
