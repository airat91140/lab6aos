#include "server_utils.h"

int serv_log(logger_msg_type_e severity, char header[LOGMSG_HEADER_SZ], char text[LOGMSG_TEXT_SZ]) {
    int msq_id = msgget(MSQ_ID, 0);
    logger_msg err_msg;
    err_msg.type = LOGGER_MSGTYP;
    err_msg.severity = severity;
    strcpy(err_msg.header, header);
    strcpy(err_msg.text, text);
    msgsnd(msq_id, &err_msg, sizeof(err_msg) - sizeof(long), 0);
}

int lock_mem(int byte_id) {
    int sem_id = semget(SEM_ID, sizeof(shared_mem), 0);
    struct sembuf sem_lock = {byte_id, -1, 0};
    return semop(sem_id, &sem_lock, 1);
}

int unlock_mem(int byte_id) {
    int sem_id = semget(SEM_ID, sizeof(shared_mem), 0);
    struct sembuf sem_lock = {byte_id, +1, 0};
    return semop(sem_id, &sem_lock, 1);
}
