#include "tcp_worker.h"

int init_connection(int listen_sock_fd) {
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    shared_mem *mem = shmat(shm_id, NULL, 0);
    lock_mem(offsetof(shared_mem, port));
    serv_addr.sin_port = htons(mem->port);
    unlock_mem(offsetof(shared_mem, port));
    lock_mem(offsetof(shared_mem, listen_sock_fd));
    mem->listen_sock_fd = listen_sock_fd;
    unlock_mem(offsetof(shared_mem, listen_sock_fd));
    shmdt(mem);

    int err_cnt = 0;
    err_cnt += bind(listen_sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    err_cnt += listen(listen_sock_fd, 5);
    serv_log(SERV_MSG_INFO, "Started socket listening", strerror(errno));
    if (err_cnt < 0) {
        sleep(1);
        kill(getpid(), SIGTERM);
        return -1;
    }
    return 0;
}

int close_connection(int listen_sock_fd) {
    return close(listen_sock_fd);
}