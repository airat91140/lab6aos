#include "logger.h"

int stop_logger = 0;

void logger_sigterm_handler(int signum) {
    stop_logger = 1;
}

int logger() {
    struct sigaction sa;
    sa.sa_handler = logger_sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    struct sigaction hup_sa;
    hup_sa.sa_flags = 0;
    hup_sa.sa_handler = SIG_IGN;
    sigemptyset(&hup_sa.sa_mask);
    sigaction(SIGHUP, &hup_sa, NULL);

    int msq_id = msgget(MSQ_ID, 0);
    int err_code;
    while(!stop_logger && (err_code = logger_listen(msq_id)) == 1);
    return err_code;
}

int logger_listen(int msq_id) {
    logger_msg msg;
    int err = msgrcv(msq_id, &msg, sizeof(logger_msg) - sizeof(long), LOGGER_MSGTYP, 0);
    if (err < 0 && errno == EINTR) return 0;
    if (err < 0) return -1;
    write2log(msg);
    return 1;
}

int write2log(logger_msg msg) {
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    char string2write[1024];
    char severity_type[16];
    shared_mem *mem = shmat(shm_id, NULL, 0);
    lock_mem(offsetof(shared_mem, logfile));
    int fd = open(mem->logfile, O_CREAT | O_WRONLY, 0666);
    unlock_mem(offsetof(shared_mem, logfile));
    lseek(fd, 0, SEEK_END);
    shmdt(mem);
    switch (msg.severity) {
        case SERV_MSG_ERROR:
            strcpy(severity_type, "ERROR");
        break;
        case SERV_MSG_WARNING:
            strcpy(severity_type, "WARN");
        break;
        case SERV_MSG_INFO:
            strcpy(severity_type, "INFO");
        break;
        default:
            strcpy(severity_type, "UNKNOWN");
        break;
    }
    int num2write = sprintf(string2write, "LAB6AOS: [%s]: %s - %s", severity_type, msg.header, msg.text);
    write(fd, string2write, num2write);
    write(fd, "\n", 1);
    close(fd);
    return 0;
}