#include <sys/types.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "../server/server_utils.h"

#define SERV_PORT 12345

#define VARIANT_SZ 64

char variants[][VARIANT_SZ] = {"0. Exit", "1. Send value", "2. Get status", "3. Get results of game"};

int init_connection(char *argv[]) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERV_PORT);
    connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    tcp_client_msg init_msg;
    init_msg.msg_type = strcmp(argv[1], "start") ? TCP_CLIENT_JOIN : TCP_CLIENT_START;
    strcpy(init_msg.password, argv[2]);
    write(sock_fd, &init_msg, sizeof(init_msg));

    return sock_fd;
}

void print_variants() {
    for (int i = 0; i < sizeof(variants) / VARIANT_SZ; ++i) {
        puts(variants[i]);
    }
}

void send_val(int sock_fd) {
    int32_t val;
    printf("Enter your val ");
    scanf("%d", &val);
    write(sock_fd, &val, sizeof(int32_t));
}

void get_status(int sock_fd) {
    char buf;
    int err = recv(sock_fd, &buf, 1, MSG_DONTWAIT);
    if (err < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        puts("No status to recieve");
        return;
    }
    switch (buf) {
        case TCP_CLIENT_STATUS_OK:
            puts("Recieved Ok");
            break;
        case TCP_CLIENT_STATUS_SERVER_SHUTTING_DOWN:
            puts("Server shtting down");
            break;
        case TCP_CLIENT_STATUS_NO_GAME:
            puts("Error: No game to join with your password");
            break;
        case TCP_CLIENT_STATUS_GAME_EXISTS:
            puts("Error: Game with provided password already exists");
            break;
        case TCP_CLIENT_STATUS_QUEUE_FULL:
            puts("Error: Sorry, but there is a maximum number of not started games");
            break;
        case TCP_CLIENT_STATUS_INVALID_COMMAND:
            puts("You've sent invalid command to server");
            break;
        case TCP_CLIENT_STATUS_NOT_YOUR_TURN:
            puts("Hey, its not your turn!");
            break;
        case TCP_CLIENT_STATUS_ALREADY_FINISHED:
            puts("You already guesed the num. Please wait for answer of your enemy");
            break;
        case TCP_CLIENT_STATUS_MORE:
            puts("The number is bigger then yours!");
            break;
        case TCP_CLIENT_STATUS_LESS:
            puts("The number is less then yours!");
            break;
        case TCP_CLIENT_STATUS_EQUALS:
            puts("You guessed the number!");
            break;
        default:
            puts("Error, Invalid status");
            break;
    }
}

void get_message(int sock_fd) {
    char buf[RESULT_MCG_SZ] = {0};
    int err = recv(sock_fd, buf, sizeof(buf), MSG_DONTWAIT);
    if (err < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        puts("No result to recieve");
        return;
    }
    puts(buf);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        puts("Invalid number of arguments");
        puts("Usage: a.out (start|join) <password>");
        return -1;
    }
    if (strcmp(argv[1], "start") && strcmp(argv[1], "join")) {
        puts("Invalid command");
        puts("Usage: a.out (start|join) <password>");
        return -1;
    }
    int sock_fd = init_connection(argv);
    int is_exit = 0;
    while (!is_exit) {
        int command;
        print_variants();
        printf("Enter your command: ");
        scanf("%d", &command);
        switch (command) {
            case 0:
                is_exit = 1;
                break;
            case 1:
                send_val(sock_fd);
                break;
            case 2:
                get_status(sock_fd);
                break;
            case 3:
                get_message(sock_fd);
                break;
            default:
                break;
        }
        perror("");
        errno = 0;
    }
    close(sock_fd);
}