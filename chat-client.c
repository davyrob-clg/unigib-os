// client.c
// Simple chat client that connects to server and reads user input
// Compile: gcc -Wall -O2 -o client client.c
// Run: ./client <host> [port]
// Default port: 12345

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>

#define DEFAULT_PORT "12345"
#define BUF_SZ 4096

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host> [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *host = argv[1];
    const char *port = (argc > 2) ? argv[2] : DEFAULT_PORT;

    struct addrinfo hints, *res, *p;
    int sockfd = -1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (sockfd == -1) {
        fprintf(stderr, "Unable to connect to %s:%s\n", host, port);
        return 2;
    }

    printf("Connected to %s:%s. Type messages and press Enter to send. Ctrl+C to quit.\n", host, port);

    fd_set read_fds;
    int fd_stdin = fileno(stdin);
    char sendbuf[BUF_SZ];
    char recvbuf[BUF_SZ];

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(fd_stdin, &read_fds);
        FD_SET(sockfd, &read_fds);
        int maxfd = (fd_stdin > sockfd) ? fd_stdin : sockfd;

        int ready = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(fd_stdin, &read_fds)) {
            // read a line from stdin
            if (fgets(sendbuf, sizeof sendbuf, stdin) == NULL) {
                // EOF or error - exit
                printf("Input closed, exiting.\n");
                break;
            }
            size_t len = strlen(sendbuf);
            if (len == 0) continue;
            // send to server
            ssize_t sent = 0;
            while (sent < (ssize_t)len) {
                ssize_t n = send(sockfd, sendbuf + sent, len - sent, 0);
                if (n == -1) {
                    perror("send");
                    goto end;
                }
                sent += n;
            }
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            ssize_t nbytes = recv(sockfd, recvbuf, sizeof recvbuf - 1, 0);
            if (nbytes <= 0) {
                if (nbytes == 0) {
                    printf("Server closed connection.\n");
                } else {
                    perror("recv");
                }
                break;
            }
            recvbuf[nbytes] = '\0';
            // print incoming message
            fputs(recvbuf, stdout);
            fflush(stdout);
        }
    }

end:
    close(sockfd);
    return 0;
}

