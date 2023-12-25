/**
 * @file server_utils.h
 * @author Makhmutov Airat
 * @brief Header file with common useful functions
 * @version 0.1
 * @date 2023-12-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef BD4A00D6_3CD8_45FB_BA21_E225C0DE4F50
#define BD4A00D6_3CD8_45FB_BA21_E225C0DE4F50

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/sem.h>
#include <stddef.h>

#define LOGFILENAME_SZ 64

#define MASTER_MSGTYP       1
#define LOGGER_MSGTYP       2
#define TCP_LISTENER_MSGTYP 3
#define RANDOMIZATOR_MSGTYP 4
#define CL_LISTENER_MSGTYP  5
#define ROUND_WORKER_MSGTYP 6

// Keys for message queue, semaphore, shared memory
#define MSQ_ID 2000
#define SEM_ID 2001
#define SHM_ID 2002

#define LOGMSG_HEADER_SZ 64
#define LOGMSG_TEXT_SZ   512

#define TCP_CLIENT_PASSWORD_SZ 64

#define CLIENTS_SZ 16

#define RESULT_MCG_SZ 256

#define SERV_RAND_MAX 1000
#define SERV_RAND_MIN 0

typedef enum {
    SERV_MSG_ERROR, SERV_MSG_WARNING, SERV_MSG_INFO
} logger_msg_type_e;

typedef enum {
    TCP_CLIENT_START = 0, TCP_CLIENT_JOIN = 1
} tcp_client_msg_type_e;

typedef enum {
    TCP_CLIENT_STATUS_OK, TCP_CLIENT_STATUS_SERVER_SHUTTING_DOWN, TCP_CLIENT_STATUS_NO_GAME, TCP_CLIENT_STATUS_GAME_EXISTS, 
    TCP_CLIENT_STATUS_QUEUE_FULL, TCP_CLIENT_STATUS_INVALID_COMMAND, TCP_CLIENT_STATUS_NOT_YOUR_TURN, 
    TCP_CLIENT_STATUS_ALREADY_FINISHED, TCP_CLIENT_STATUS_MORE, TCP_CLIENT_STATUS_LESS, TCP_CLIENT_STATUS_EQUALS
} tcp_client_status_e;

typedef struct logger_msg {
    long type;
    logger_msg_type_e severity;
    char header[LOGMSG_HEADER_SZ];
    char text[LOGMSG_TEXT_SZ];
} logger_msg;

typedef struct tcp_client_msg {
    char msg_type;
    char password[TCP_CLIENT_PASSWORD_SZ];
} tcp_client_msg;

// message to server with user credits
typedef struct tcp_worker_msg {
    long type;
    tcp_client_msg tcp_recieved;
    int client_fd;
} tcp_worker_msg;

typedef struct randomizator_msg {
    long type;
    int rand_num;
} randomizator_msg;

typedef struct cl_listener_msg {
    long type;
    char status;
    int val;
    int fd;
    int cl_id;
} cl_listener_msg;

typedef struct round_worker_msg {
    long type;
    int val;
    int cl_id;
    int fd;
} round_worker_msg;

typedef struct master_result_msg {
    long type;
    int fd;
    char status;
} master_result_msg;

typedef struct shared_mem {
    int port;
    char logfile[LOGFILENAME_SZ];
    tcp_worker_msg clients[CLIENTS_SZ];
    int clients_len;
    int32_t random_num;
    int cl_atmpt[2];
    char cl_turn; // 0 or 1
    int cl_finished[2];
    int cl_finished_after[2];
    int cl_fd[2];
    int listen_sock_fd;
    int end_of_game;
} shared_mem;

/**
 * @brief log some information or error
 * 
 * @param severity type of log message. See logger_msg_type_e
 * @param header header for log message
 * @param text body message of log
 * @return int error code. Negative on error
 */
int serv_log(logger_msg_type_e severity, char header[LOGMSG_HEADER_SZ], char text[LOGMSG_TEXT_SZ]);

/**
 * @brief lock memory with semaphore
 * 
 * @param byte_id number of first byte of locked field in memory struct
 * @return int error code. Negative on error
 */
int lock_mem(int byte_id);

/**
 * @brief unlock memory with semaphore
 * 
 * @param byte_id number of first byte of unlocked field in memory struct
 * @return int error code. Negative on error
 */
int unlock_mem(int byte_id);

#endif /* BD4A00D6_3CD8_45FB_BA21_E225C0DE4F50 */
