#include "master.h"

int stop_server = 0;

void sigterm_handler(int signum) {
    serv_log(SERV_MSG_WARNING, "Master", "SIGTERM recieved, killing ourselves");
    stop_server = 1;
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    mem->end_of_game = 1;
    mem->cl_finished[0] = 1;
    mem->cl_finished[1] = 1;
    mem->cl_finished_after[0] = 1;
    mem->cl_finished_after[1] = 1;
    shmdt(mem);
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    while (wait(NULL) > 0);
}

void sighup_handler(int signum) {
    config_update();
}

int master() {
    if (init() < 0) return -1;
    if (config_update() < 0) return -1;
    int logger_pid = fork();
    if (logger_pid == 0) exit(logger());
    serv_log(SERV_MSG_INFO, "Logger", "logger started");
    int listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    init_connection(listen_sock_fd);
    listen_requests(listen_sock_fd);
    die();
}

int listen_requests(int listen_sock_fd) {
    int msq_id = msgget(MSQ_ID, 0);
    int read_fd;
    while(!stop_server) {
        tcp_worker_msg client_info;
        read_fd = accept(listen_sock_fd, NULL, 0);
        tcp_client_msg cl_msg;
        recv(read_fd, &cl_msg, sizeof(tcp_client_msg), MSG_WAITALL);
        if (stop_server) break;
        client_info.tcp_recieved = cl_msg;
        client_info.client_fd = read_fd;
        serv_log(SERV_MSG_INFO, "Master", "Request to start/join recieved");
        int hostfd;
        check_game(client_info, &hostfd);
        if (hostfd >= 0) { // starting a game
            start_game(hostfd, client_info.client_fd);
        }
    }
    return 0;
}

int init() {
    int err_cnt = 0;
    int msq_id = msgget(MSQ_ID, IPC_CREAT | 0666);
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0666 | IPC_CREAT);
    int sem_id = semget(SEM_ID, sizeof(shared_mem), 0666 | IPC_CREAT);

    err_cnt += msq_id < 0 ? -1 : 0;
    err_cnt += shm_id < 0 ? -1 : 0;
    err_cnt += sem_id < 0 ? -1 : 0;

    short sem_array[sizeof(shared_mem)];
    memset(sem_array, 1, sizeof(sem_array));

    semctl(sem_id, 0, SETALL, sem_array);

    shared_mem *mem = shmat(shm_id, NULL, 0);
    mem->clients_len = 0; // no need to lock because no other processes
    shmdt(mem);

    struct sigaction term_sa;
    term_sa.sa_handler = sigterm_handler;
    term_sa.sa_flags = 0;
    sigemptyset(&term_sa.sa_mask);
    sigaction(SIGTERM, &term_sa, NULL);

    struct sigaction hup_sa;
    hup_sa.sa_handler = sighup_handler;
    hup_sa.sa_flags = SA_RESTART;
    sigemptyset(&hup_sa.sa_mask);
    sigaction(SIGHUP, &hup_sa, NULL);

    return err_cnt;
}

int die() {
    int msq_id = msgget(MSQ_ID, 0);
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    int sem_id = semget(SEM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    close(mem->listen_sock_fd);
    for (int i = 0; i < mem->clients_len; ++i) {
        close(mem->clients->client_fd);
    }
    shmdt(mem);

    msgctl(msq_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    shmctl(shm_id, IPC_RMID, NULL);
}

int check_game(tcp_worker_msg client, int *hostfd) {
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    if (mem->clients_len == CLIENTS_SZ && client.tcp_recieved.msg_type == TCP_CLIENT_START) {
        char cl_status = TCP_CLIENT_STATUS_QUEUE_FULL;
        write(client.client_fd, &cl_status, sizeof(char));
        close(client.client_fd);
        *hostfd = -1;
        return 0;
    }
    int i;
    for (i = 0; i < mem->clients_len; ++i) {
        if(!strcmp(client.tcp_recieved.password, mem->clients[i].tcp_recieved.password)) {
            if(client.tcp_recieved.msg_type == TCP_CLIENT_START) {
                serv_log(SERV_MSG_WARNING, "Master", "Game already exists");
                char cl_status = TCP_CLIENT_STATUS_GAME_EXISTS;
                write(client.client_fd, &cl_status, sizeof(char));
                close(client.client_fd);
                *hostfd = -1;
                return 0;
            }
            if(client.tcp_recieved.msg_type == TCP_CLIENT_JOIN) {
                serv_log(SERV_MSG_INFO, "Master", "Join accepted");
                char cl_status = TCP_CLIENT_STATUS_OK;
                write(client.client_fd, &cl_status, sizeof(char));
                *hostfd = mem->clients[i].client_fd;
                lock_mem(offsetof(shared_mem, clients));
                for (int j = i; j < (mem->clients_len - 1); ++j) {
                    mem->clients[j] = mem->clients[j+1];
                }
                unlock_mem(offsetof(shared_mem, clients));
                lock_mem(offsetof(shared_mem, clients_len));
                mem->clients_len--;
                unlock_mem(offsetof(shared_mem, clients_len));
                return 0;
            }
            serv_log(SERV_MSG_ERROR, "Master", "Recieved unknow command");
            *hostfd = -1;
            char cl_status = TCP_CLIENT_STATUS_INVALID_COMMAND;
            write(client.client_fd, &cl_status, sizeof(char));
            close(client.client_fd);
            return 0; // did not found msg_type
        }
    }
    if(client.tcp_recieved.msg_type == TCP_CLIENT_START) {
        serv_log(SERV_MSG_INFO, "Master", "Game created, waiting to join");
        lock_mem(offsetof(shared_mem, clients));
        lock_mem(offsetof(shared_mem, clients_len));
        mem->clients[mem->clients_len++] = client;
        unlock_mem(offsetof(shared_mem, clients_len));
        unlock_mem(offsetof(shared_mem, clients));
        char cl_status = TCP_CLIENT_STATUS_OK;
        send(client.client_fd, &cl_status, sizeof(char), 0);
        *hostfd = -1;
        return 0;
    }
    if(client.tcp_recieved.msg_type == TCP_CLIENT_JOIN) {
        serv_log(SERV_MSG_WARNING, "Master", "Game to join not found");
        char cl_status = TCP_CLIENT_STATUS_NO_GAME;
        write(client.client_fd, &cl_status, sizeof(char));
        *hostfd = -1;
        close(client.client_fd);
        return 0;
    }
    serv_log(SERV_MSG_ERROR, "Master", "Recieved unknow command");
    *hostfd = -1;
    char cl_status = TCP_CLIENT_STATUS_INVALID_COMMAND;
    write(client.client_fd, &cl_status, sizeof(char));
    close(client.client_fd);
    return 0;
}

int start_game(int client1, int client2) {
    int rand_pid = fork();
    if (rand_pid == 0) exit(randomization(SERV_RAND_MIN, SERV_RAND_MAX));
    if (stop_server) {
        char buf = TCP_CLIENT_STATUS_SERVER_SHUTTING_DOWN;
        write(client1, &buf, 1);
        write(client2, &buf, 1);
        close(client1);
        close(client2);
        return 0;
    }
    waitpid(rand_pid, NULL, 0);
    shootout(client1, client2);
    close(client1);
    close(client2);
    serv_log(SERV_MSG_INFO, "Master", "End of game");
    return 0;
}

int shootout(int fd1, int fd2) {
    serv_log(SERV_MSG_INFO, "Master", "SHOOTOUT began");

    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);

    lock_mem(offsetof(shared_mem, cl_fd));
    mem->cl_fd[0] = fd1;
    mem->cl_fd[1] = fd2;
    unlock_mem(offsetof(shared_mem, cl_fd));

    int cl1_listener_pid = fork();
    if (cl1_listener_pid == 0) exit(client_listener(0, fd1));
    int cl2_listener_pid = fork();
    if (cl2_listener_pid == 0) exit(client_listener(1, fd2));

    while (!mem->end_of_game) {
        int rnd_pid = fork();
        if (rnd_pid == 0) exit(round_worker());
        if (!mem->cl_finished[0])
            listener_processing();
        if (!mem->cl_finished[1])
            listener_processing();
        waitpid(rnd_pid, NULL, 0);
        send_hints(mem);
        serv_log(SERV_MSG_INFO, "Master", "Round ended");
    }
    if (mem->end_of_game) {
        char buf = TCP_CLIENT_STATUS_SERVER_SHUTTING_DOWN;
        write(fd1, &buf, 1);
        write(fd2, &buf, 1);
    }
    shmdt(mem);
}

int listener_processing() {
    int msq_id = msgget(MSQ_ID, 0);
    cl_listener_msg msg;
    while (!stop_server) {
        int res;
        do {
            if (stop_server) break;
            res = msgrcv(msq_id, &msg, sizeof(msg) - sizeof(long), CL_LISTENER_MSGTYP, 0);
            if (stop_server) break;
        } while(res < 0 && errno == EINTR); // sighup issue
        if (stop_server) break;
        if (msg.status == TCP_CLIENT_STATUS_OK) {
            round_worker_msg rnd_msg;
            rnd_msg.type = ROUND_WORKER_MSGTYP;
            rnd_msg.cl_id = msg.cl_id;
            rnd_msg.val = msg.val;
            rnd_msg.fd = msg.fd;
            msgsnd(msq_id, &rnd_msg, sizeof(rnd_msg) - sizeof(long), 0);
            break;
        } else {
            char buf = msg.status;
            write(msg.fd, &buf, 1);
        }
    }
    return 0;
}

int send_hints(shared_mem *mem) {
    int msq_id = msgget(MSQ_ID, 0);
    master_result_msg res_msg;
    if (!mem->cl_finished_after[0]) {
        msgrcv(msq_id, &res_msg, sizeof(res_msg) - sizeof(long), MASTER_MSGTYP, 0);
        if (res_msg.status == TCP_CLIENT_STATUS_EQUALS) {
            lock_mem(offsetof(shared_mem, cl_finished_after));
            mem->cl_finished_after[mem->cl_fd[1] == res_msg.fd] = 1;
            unlock_mem(offsetof(shared_mem, cl_finished_after));
        }
        if (stop_server) return 0;
        write(res_msg.fd, &res_msg.status, 1);
    }
    if (!mem->cl_finished_after[1]) {
        msgrcv(msq_id, &res_msg, sizeof(res_msg) - sizeof(long), MASTER_MSGTYP, 0);
        if (res_msg.status == TCP_CLIENT_STATUS_EQUALS) {
            lock_mem(offsetof(shared_mem, cl_finished_after));
            mem->cl_finished_after[mem->cl_fd[1] == res_msg.fd] = 1;
            unlock_mem(offsetof(shared_mem, cl_finished_after));
        }
        if (stop_server) return 0;
        write(res_msg.fd, &res_msg.status, 1);
    }
    serv_log(SERV_MSG_INFO, "Hints", "Answered to players with hints");
    return 0;
}