#include "randomizator.h"

void rnd_sigterm_handler(int signum) {
    return;
}

int randomization(int min_num, int max_num) {
    struct sigaction sa;
    sa.sa_handler = rnd_sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    struct sigaction hup_sa;
    hup_sa.sa_flags = 0;
    hup_sa.sa_handler = SIG_IGN;
    sigemptyset(&hup_sa.sa_mask);
    sigaction(SIGHUP, &hup_sa, NULL);

    int rand_fd = open("/dev/urandom", O_RDONLY);
    int rand_num;
    read(rand_fd, &rand_num, sizeof(int));
    rand_num = rand_num < 0 ? -rand_num : rand_num;
    rand_num = (rand_num % (max_num - min_num)) + min_num;
    
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    shared_mem *mem = shmat(shm_id, NULL, 0);
    lock_mem(offsetof(shared_mem, random_num));
    mem->random_num = rand_num;
    unlock_mem(offsetof(shared_mem, random_num));
    lock_mem(offsetof(shared_mem, cl_atmpt));
    memset(mem->cl_atmpt, 0, sizeof(mem->cl_atmpt));
    unlock_mem(offsetof(shared_mem, cl_atmpt));
    lock_mem(offsetof(shared_mem, cl_finished));
    memset(mem->cl_finished, 0, sizeof(mem->cl_finished));
    unlock_mem(offsetof(shared_mem, cl_finished));
    lock_mem(offsetof(shared_mem, cl_finished_after));
    memset(mem->cl_finished_after, 0, sizeof(mem->cl_finished_after));
    unlock_mem(offsetof(shared_mem, cl_finished_after));
    lock_mem(offsetof(shared_mem, cl_turn));
    mem->cl_turn = 0;
    unlock_mem(offsetof(shared_mem, cl_turn));
    lock_mem(offsetof(shared_mem, end_of_game));
    mem->end_of_game = 0;
    unlock_mem(offsetof(shared_mem, end_of_game));
    shmdt(mem);
    char buf[128];
    sprintf(buf, "randomiztion succeeded, rnd=%d", rand_num);
    serv_log(SERV_MSG_INFO, "Randomizator", buf);
    return 0;
}
