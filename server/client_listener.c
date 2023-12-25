#include "client_listener.h"

void lstn_sigterm_handler(int signum) {
    return;
}

int client_listener(int cl_id, int fd) {
    struct sigaction sa;
    sa.sa_handler = lstn_sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    struct sigaction hup_sa;
    hup_sa.sa_flags = 0;
    hup_sa.sa_handler = SIG_IGN;
    sigemptyset(&hup_sa.sa_mask);
    sigaction(SIGHUP, &hup_sa, NULL);

    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    while (!mem->end_of_game) {
        int32_t cl_val;
        int err = recv(fd, &cl_val, sizeof(int32_t), MSG_WAITALL);
        if (err < 0 && errno == EINTR) {
            shmdt(mem);
            return 0;
        }
        if (err <= 0) {
            lock_mem(offsetof(shared_mem, end_of_game));
            mem->end_of_game = 1;
            unlock_mem(offsetof(shared_mem, end_of_game));
            shmdt(mem);
            return -1;
        }
        serv_log(SERV_MSG_INFO, "Listener", "Recieved message from client");
        if (mem->cl_finished_after[cl_id]) {
            cl_listener_msg msg;
            msg.type = CL_LISTENER_MSGTYP;
            msg.status = TCP_CLIENT_STATUS_ALREADY_FINISHED;
            msg.fd = fd;
            msg.cl_id = cl_id;
            int msq_id = msgget(MSQ_ID, 0);
            msgsnd(msq_id, &msg, sizeof(msg) - sizeof(long), 0);
            continue;
        }
        if (cl_id != mem->cl_turn) {
            cl_listener_msg msg;
            msg.type = CL_LISTENER_MSGTYP;
            msg.status = TCP_CLIENT_STATUS_NOT_YOUR_TURN;
            msg.fd = fd;
            msg.cl_id = cl_id;
            int msq_id = msgget(MSQ_ID, 0);
            msgsnd(msq_id, &msg, sizeof(msg) - sizeof(long), 0);
            continue;
        }
        cl_listener_msg msg;
        msg.type = CL_LISTENER_MSGTYP;
        msg.status = TCP_CLIENT_STATUS_OK;
        msg.fd = fd;
        msg.val = cl_val;
        msg.cl_id = cl_id;
        int msq_id = msgget(MSQ_ID, 0);
        msgsnd(msq_id, &msg, sizeof(msg) - sizeof(long), 0);
        lock_mem(offsetof(shared_mem, cl_turn));
        if (!mem->cl_finished[!cl_id])
            mem->cl_turn = !cl_id;
        unlock_mem(offsetof(shared_mem, cl_turn));
    }
    shmdt(mem);
    return 0;
}