#include "configger.h"

int config_update() {
    serv_log(SERV_MSG_INFO, "Config", "Config file readen");
    int shm_id = shmget(SHM_ID, sizeof(shared_mem), 0);
    int err_cnt = 0;
    char conf_buf[CONFBUF_SZ];
    int fd = open(config_filename, O_RDONLY, 0666);
    if (fd < 0)
        return -1;
    shared_mem *mem = shmat(shm_id, NULL, 0);
    err_cnt += parse_config(mem, fd);
    shmdt(mem);
    close(fd);
    return err_cnt;
}

int parse_config(shared_mem *mem, int fd) {
    char line_buf[LINEBUF_SZ];
    int n;
    while (((n = read_line(line_buf, fd)) > 0) || (n == -1)) {
        if (line_buf[0] == '#') // skipping comment
            continue;
        if (n == -1) {// line larger then buf
            char mes_text[LOGMSG_TEXT_SZ];
            snprintf(mes_text, LOGMSG_TEXT_SZ, "Unknown config option %s", line_buf);
            serv_log(SERV_MSG_ERROR, "Parsing config", mes_text);
            while (read_line(line_buf, fd) == -1); // skipping a whole line
        }
        char key[256];
        strcpy(key, strtok(line_buf, "="));
        if (strcmp(key, "logfile") == 0) {
            lock_mem(offsetof(shared_mem, logfile));
            strcpy(mem->logfile, strtok(NULL, "="));
            sprintf(mem->logfile, "%s%ld", mem->logfile, time(NULL));
            unlock_mem(offsetof(shared_mem, logfile));
        } else if (strcmp(key, "port") == 0) {
            char tmpbuf[16];
            strcpy(tmpbuf, strtok(NULL, "="));
            lock_mem(offsetof(shared_mem, port));
            mem->port = strtoll(tmpbuf, NULL, 10);
            unlock_mem(offsetof(shared_mem, port));
        } else {
            char mes_text[LOGMSG_TEXT_SZ];
            snprintf(mes_text, LOGMSG_TEXT_SZ, "Unknown key %s", line_buf);
            serv_log(SERV_MSG_ERROR, "Parsing config", mes_text);
        }
    }
    if (n == -2) {// error while reading
        char mes_text[LOGMSG_TEXT_SZ];
        snprintf(mes_text, LOGMSG_TEXT_SZ, "Error while reading: %s", strerror(errno));
        serv_log(SERV_MSG_ERROR, "Parsing config", mes_text);
        return -1;
    }
    return 0;
}

int read_line(char line_buf[LINEBUF_SZ], int fd) {
    int n;
    char buf[1];
    int index = 0;
    while ((n = read(fd, buf, 1)) > 0) {
        if (*buf == ' ' || *buf == '\t') // skipping forward spaces
            continue;
        if (*buf == '\n') {
            line_buf[index] = '\0';
            break;
        }
        line_buf[index++] = *buf;
        if (index >= LINEBUF_SZ)
            return -1;
    }
    return n >= 0 ? index : -2;
}