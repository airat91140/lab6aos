#include "daemon.h"

int launch_daemon() {
    int err_cnt = 0;
    close_fds();
    err_cnt += chdir("/");
    err_cnt += run_bg();
    err_cnt += setsid();
    return err_cnt;
}

void close_fds() {
    for (int fd = 0; fd < 3; fd++) {
        close(fd);
    }  
}

int run_bg() {
    int f = fork();
    if (f < 0)
        return -1;
    if (f != 0) {
        exit(0);
    }
    return f;
}