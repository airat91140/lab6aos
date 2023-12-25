#include "round_worker.h"

void round_sigterm_handler(int signum) {
    return;
}

int round_worker() {
    struct sigaction sa;
    sa.sa_handler = round_sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    struct sigaction hup_sa;
    hup_sa.sa_flags = 0;
    hup_sa.sa_handler = SIG_IGN;
    hup_sa.sa_flags = 0;
    sigemptyset(&hup_sa.sa_mask);
    sigaction(SIGHUP, &hup_sa, NULL);

    int msq_id = msgget(MSQ_ID, 0);
    round_worker_msg rnd_msg[2];
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    int err = msgrcv(msq_id, rnd_msg, sizeof(round_worker_msg) - sizeof(long), ROUND_WORKER_MSGTYP, 0);
    if (err < 0 && errno == EINTR) return 0;
    check_cmp(rnd_msg[0]);
    lock_mem(offsetof(shared_mem, cl_atmpt));
    mem->cl_atmpt[rnd_msg->cl_id]++;
    unlock_mem(offsetof(shared_mem, cl_atmpt));
    if (!mem->cl_finished_after[0] && !mem->cl_finished_after[1]) {
        err = msgrcv(msq_id, rnd_msg + 1, sizeof(round_worker_msg) - sizeof(long), ROUND_WORKER_MSGTYP, 0);
        if (err < 0 && errno == EINTR) return 0;
        check_cmp(rnd_msg[1]);
        lock_mem(offsetof(shared_mem, cl_atmpt));
        mem->cl_atmpt[rnd_msg[1].cl_id]++;
        unlock_mem(offsetof(shared_mem, cl_atmpt));
    }
    if (mem->end_of_game == 0 && mem->cl_finished[0] && mem->cl_finished[1]) {
        master_result_msg toremove;
        msgrcv(msq_id, &toremove, sizeof(toremove) - sizeof(long), MASTER_MSGTYP, IPC_NOWAIT);
        msgrcv(msq_id, &toremove, sizeof(toremove) - sizeof(long), MASTER_MSGTYP, IPC_NOWAIT);
        lock_mem(offsetof(shared_mem, end_of_game));
        mem->end_of_game = 1;
        unlock_mem(offsetof(shared_mem, end_of_game));
        send_results();
    }
    shmdt(mem);
    return 0;
}

int check_cmp(round_worker_msg rnd_msg) {
    int msq_id = msgget(MSQ_ID, 0);
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);

    master_result_msg res_msg;
    res_msg.type = MASTER_MSGTYP;
    res_msg.fd = rnd_msg.fd;
    if (mem->random_num > rnd_msg.val) {
        res_msg.status = TCP_CLIENT_STATUS_MORE;
    }
    else if (mem->random_num < rnd_msg.val) {
        res_msg.status = TCP_CLIENT_STATUS_LESS;
    }
    else {
        res_msg.status = TCP_CLIENT_STATUS_EQUALS;
        lock_mem(offsetof(shared_mem, cl_finished));
        mem->cl_finished[rnd_msg.cl_id] = 1;
        unlock_mem(offsetof(shared_mem, cl_finished));
    }
    msgsnd(msq_id, &res_msg, sizeof(res_msg) - sizeof(long), 0);
    shmdt(mem);
    return 0;
}

int send_results() {
    char buf[RESULT_MCG_SZ];
    int winner_fd, loser_fd;
    int winner_atmpts, loser_atmpts;
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    if (mem->cl_atmpt[0] > mem->cl_atmpt[1]) {
        winner_fd = mem->cl_fd[1];
        loser_fd = mem->cl_fd[0];
        winner_atmpts = mem->cl_atmpt[1];
        loser_atmpts = mem->cl_atmpt[0];
    } else if (mem->cl_atmpt[0] < mem->cl_atmpt[1]) {
        winner_fd = mem->cl_fd[0];
        loser_fd = mem->cl_fd[1];
        winner_atmpts = mem->cl_atmpt[0];
        loser_atmpts = mem->cl_atmpt[1];
    } else {
        lock_mem(offsetof(shared_mem, cl_finished_after));
        mem->cl_finished_after[0] = 1;
        mem->cl_finished_after[1] = 1;
        unlock_mem(offsetof(shared_mem, cl_finished_after));
        char eq = TCP_CLIENT_STATUS_EQUALS;
        write(mem->cl_fd[0], &eq, 1);
        write(mem->cl_fd[1], &eq, 1);
        sprintf(buf, "Friendship has won: %d attempts", mem->cl_atmpt[0]);
        write(mem->cl_fd[0], buf, strlen(buf) + 1);
        write(mem->cl_fd[1], buf, strlen(buf) + 1);
        shmdt(mem);
        return 0;
    }
    char eq = TCP_CLIENT_STATUS_EQUALS;
    write(loser_fd, &eq, 1);
    sprintf(buf, "Winner, winner chicken dinner: %d attempts", winner_atmpts);
    write(winner_fd, buf, sizeof(buf));
    sprintf(buf, "Better luck next time: %d attempts", loser_atmpts);
    write(loser_fd, buf, sizeof(buf));

    lock_mem(offsetof(shared_mem, cl_finished_after));
    mem->cl_finished_after[0] = 1;
    mem->cl_finished_after[1] = 1;
    unlock_mem(offsetof(shared_mem, cl_finished_after));

    shmdt(mem);
    return 0;
}