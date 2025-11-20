// server.c
// Simple multi-client chat server using select()
// Compile: gcc -Wall -O2 -o server server.c
// Run: ./server [port]
// Default port: 12345

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BACKLOG 10
#define BUF_SZ 4096
#define DEFAULT_PORT "12345"
#define MAX_CLIENTS FD_SETSIZE

int max(int a, int b){ return a>b? a:b; }

int setup_listen(const char *port) {
    struct addrinfo hints, *res, *p;
    int listenfd = -1;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for(p = res; p != NULL; p = p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd == -1) continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(listenfd);
            continue;
        }

        if (listen(listenfd, BACKLOG) == -1) {
            perror("listen");
            close(listenfd);
            freeaddrinfo(res);
            return -1;
        }

        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "Failed to bind/listen on port %s\n", port);
        return -1;
    }

    return listenfd;
}

void broadcast(int sender_fd, int *clients, int max_clients, const char *msg, ssize_t msglen) {
    for (int i = 0; i < max_clients; ++i) {
        int fd = clients[i];
        if (fd != -1 && fd != sender_fd) {
            ssize_t sent = 0;
            while (sent < msglen) {
                ssize_t n = send(fd, msg + sent, msglen - sent, 0);
                if (n <= 0) {
                    // can't handle here; client will be cleaned up on recv error
                    break;
                }
                sent += n;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    const char *port = (argc > 1) ? argv[1] : DEFAULT_PORT;
    int listener = setup_listen(port);
    if (listener < 0) exit(EXIT_FAILURE);

    printf("Listening on port %s\n", port);

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(listener, &master);
    int fdmax = listener;

    int clients[MAX_CLIENTS];
    int client_id[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; ++i) { clients[i] = -1; client_id[i] = -1; }
    int next_id = 1;

    while (1) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // check all fds
        for (int fd = 0; fd <= fdmax; ++fd) {
            if (!FD_ISSET(fd, &read_fds)) continue;

            if (fd == listener) {
                // new connection
                struct sockaddr_storage remoteaddr;
                socklen_t addrlen = sizeof remoteaddr;
                int newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
                if (newfd == -1) {
                    perror("accept");
                    continue;
                }

                // add to clients array
                int slot = -1;
                for (int i = 0; i < MAX_CLIENTS; ++i) {
                    if (clients[i] == -1) { slot = i; break; }
                }
                if (slot == -1) {
                    const char *msg = "Server full, try later.\n";
                    send(newfd, msg, strlen(msg), 0);
                    close(newfd);
                    continue;
                }

                clients[slot] = newfd;
                client_id[slot] = next_id++;
                FD_SET(newfd, &master);
                if (newfd > fdmax) fdmax = newfd;

                // greet and announce
                char addrstr[INET6_ADDRSTRLEN];
                void *addr;
                if (((struct sockaddr*)&remoteaddr)->sa_family == AF_INET) {
                    addr = &((struct sockaddr_in*)&remoteaddr)->sin_addr;
                } else {
                    addr = &((struct sockaddr_in6*)&remoteaddr)->sin6_addr;
                }
                inet_ntop(((struct sockaddr*)&remoteaddr)->sa_family, addr, addrstr, sizeof addrstr);

                char welcome[256];
                int id = client_id[slot];
                snprintf(welcome, sizeof welcome, "Welcome! You are Client %d\n", id);
                send(newfd, welcome, strlen(welcome), 0);

                char announce[512];
                snprintf(announce, sizeof announce, "Client %d has joined from %s\n", id, addrstr);
                printf("%s", announce);
                broadcast(newfd, clients, MAX_CLIENTS, announce, strlen(announce));
            } else {
                // data from a client
                char buf[BUF_SZ];
                ssize_t nbytes = recv(fd, buf, sizeof buf, 0);
                if (nbytes <= 0) {
                    // got error or connection closed by client
                    if (nbytes == 0) {
                        // find client id
                        int id = -1;
                        for (int i = 0; i < MAX_CLIENTS; ++i) if (clients[i] == fd) { id = client_id[i]; clients[i] = -1; client_id[i] = -1; break; }
                        char msg[128];
                        if (id != -1) {
                            snprintf(msg, sizeof msg, "Client %d has disconnected\n", id);
                            printf("%s", msg);
                            broadcast(fd, clients, MAX_CLIENTS, msg, strlen(msg));
                        }
                    } else {
                        perror("recv");
                    }
                    close(fd);
                    FD_CLR(fd, &master);
                } else {
                    // got message; ensure null-terminated for printing
                    // trim and broadcast
                    // find sender id
                    int id = -1;
                    for (int i = 0; i < MAX_CLIENTS; ++i) if (clients[i] == fd) { id = client_id[i]; break; }
                    // ensure message ends with newline
                    if (nbytes > 0 && buf[nbytes-1] != '\n') {
                        // append newline in broadcast buffer
                    }
                    // prepare broadcast message: "Client N: message"
                    char outbuf[BUF_SZ + 64];
                    int outlen = snprintf(outbuf, sizeof outbuf, "Client %d: ", id);
                    int copylen = (nbytes < (int)(sizeof outbuf - outlen - 1)) ? nbytes : (int)(sizeof outbuf - outlen - 1);
                    memcpy(outbuf + outlen, buf, copylen);
                    outlen += copylen;
                    // ensure newline
                    if (outlen == 0 || outbuf[outlen-1] != '\n') {
                        outbuf[outlen++] = '\n';
                    }
                    outbuf[outlen] = '\0';
                    printf("%s", outbuf);
                    broadcast(fd, clients, MAX_CLIENTS, outbuf, outlen);
                }
            }
        } // end for fd loop
    } // end while

    close(listener);
    return 0;
}

