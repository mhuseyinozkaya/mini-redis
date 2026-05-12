#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <netinet/tcp.h>

#include "structure.h"
#include "server.h"
#include "parser.h"
#include "instructions.h"
#include "utils.h"

int get_local_addr(struct addrinfo **res, char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(NULL, port, &hints, res);
    if (status != EXIT_SUCCESS)
    {
        fprintf(stderr, "error getaddrinfo(): %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int get_listener_socket(struct addrinfo *p)
{
    int listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0)
    {
        perror("socket error");
        return -1;
    }
    return listener;
}

int bind_lsock(int sock, struct addrinfo *res)
{
    // Prevent to bind error 'Address in use'
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    // Associate socket file descriptor with an IP address and Port
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }
}

/* This function returns IP address from the given address */
char *getip_addr(struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in *in = (struct sockaddr_in *)addr;
        char *inet = (char *)calloc(1, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(in->sin_addr), inet, INET_ADDRSTRLEN);
        return inet;
    }
    else if (addr->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)addr;
        char *inet6 = (char *)calloc(1, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(in6->sin6_addr), inet6, INET6_ADDRSTRLEN);
        return inet6;
    }
    return NULL;
}

unsigned short get_port(struct sockaddr *addr)
{
    short port = 0;
    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in *in = (struct sockaddr_in *)addr;
        port = ntohs(in->sin_port);
        return port;
    }
    else if (addr->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)addr;
        port = ntohs(in6->sin6_port);
        return port;
    }
    return 0;
}

int add_to_poll(struct pollfd *p, int *index, int sock, short event, int max_size)
{
    if (*index >= max_size)
    {
        return -1;
    }
    p[*index].fd = sock;
    p[*index].events = event;
    (*index)++;
    return 0;
}

/* Closes file descriptor of the client and */
/* the last client element moves into the closed one */
int remove_from_poll(struct pollfd *pfds, int i, int *pfdscount)
{
    close(pfds[i].fd);
    pfds[i] = pfds[(*pfdscount) - 1];
    return 0;
}

int add_new_client(struct client *cl, int fd, struct sockaddr_storage cl_addr, socklen_t cl_addrlen)
{
    memset(cl, 0, sizeof(struct client));
    cl->addr = cl_addr;
    cl->addrlen = cl_addrlen;
    cl->fd = fd;
    return 0;
}

int remove_client(struct client *clients, int i, int *size)
{
    if (i != (*size - 1))
    {
        clients[i] = clients[(*size) - 1];
    }
    memset(&clients[(*size) - 1], 0, sizeof(struct client));
    // (*size)--;
    return 0;
}

void handle_new_connection(struct pollfd *pfds, int *pfdscount, struct client *clients)
{
    struct sockaddr_storage client_addr;
    socklen_t client_addrlen = sizeof client_addr;

    int clientfd = accept(pfds[0].fd, (struct sockaddr *)&client_addr, &client_addrlen);
    // TCP_NODELAY Ayarı
    int opt = 1;
    setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    if (add_to_poll(pfds, pfdscount, clientfd, (POLLIN | POLLHUP | POLLERR), SIZE_CLIENTS) == -1)
    {
        /* Liste dolduğu zaman genişletilecek */
        fprintf(stderr, "Listen Queue was full\n");
        fflush(stderr);
        return;
    }

    if (add_new_client(&clients[(*pfdscount) - 1], clientfd, client_addr, client_addrlen) != EXIT_SUCCESS)
    {
        fprintf(stderr, "Hata oluştu: add_new_client");
        return;
    }

    char *addr = getip_addr((struct sockaddr *)&client_addr);
    fprintf(stdout, "Connection recieved: %s %u, active connections: %d\n", addr, get_port((struct sockaddr *)&client_addr), (*pfdscount) - 1);
    fflush(stdout);
    free(addr);
    return;
}

int handle_response_message(struct client *cl, MSG_TYPE msg_type, const char *format, ...)
{
    char temp[B_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);

    switch (msg_type)
    {
    case ERROR:
        resp_simple(cl, "-", temp);
        break;
    case NIL:
        cl->buffer_size = snprintf(cl->send_buffer, sizeof(cl->send_buffer), "$-1\r\n");
        break;
    case INFO:
        resp_simple(cl, "+", temp);
        break;
    case BULK_STRING:
        cl->buffer_size = snprintf(cl->send_buffer, sizeof(cl->send_buffer), "%s", temp);
        break;
    default:
        break;
    }

    DEBUG_LOG("[Send log]: message: ");
    DEBUG_BUFFER(cl->send_buffer, strlen(cl->send_buffer));
    return 0;
}

int send_response(struct pollfd *pfds, struct client *cl)
{
    int ss = send(pfds->fd, cl->send_buffer, cl->buffer_size, MSG_NOSIGNAL);
    if (ss == -1)
    {
        return 1;
    }
    if (ss < cl->buffer_size)
    {
        fprintf(stderr, " <%d> bytes sent the client, the expected <%d>\n", ss, cl->buffer_size);
        return 2;
    }
    return 0;
}

int handle_disconnect(struct pollfd *pfds, int i, int *pfdscount, struct client *cls)
{
    remove_from_poll(pfds, i, pfdscount);
    remove_client(cls, i, pfdscount);
    (*pfdscount)--;
    return 0;
}
int handle_request(struct pollfd *pfds, int i, int *pfdscount, struct client *cls, Data **hash_t)
{
    cls[i].recieved_size = recv(pfds[i].fd, cls[i].recv_buf, B_SIZE, 0);
    cls[i].recv_buf[cls[i].recieved_size] = '\0';
    DEBUG_LOG("recieved_size: %d\n", cls[i].recieved_size);
    DEBUG_BUFFER(cls[i].recv_buf, cls[i].recieved_size);
    if (cls[i].recieved_size < 1)
    {
        if (handle_disconnect(pfds, i, pfdscount, cls) == EXIT_SUCCESS)
        {
            fprintf(stdout, "A client disconnected, active connection count: %d\n", (*pfdscount) - 1);
            fflush(stdout);
        }
        return 1;
    }
    int c; /* Argüman sayısı için sayaç */
    char **args = redis_tokenizer(cls[i].recv_buf, cls[i].recieved_size, &c);
    if (args == NULL)
        return -1;

    DEBUG_LOG("args count: %d\n", c);
    for (int i = 0; i < c; i++)
    {
        DEBUG_LOG("args[%d]: ", i);
        DEBUG_BUFFER(args[i], strlen(args[i]));
    }

    // Convert upper the instruction name
    to_upper(args[0]);
    // Get instruction and perform actions
    COMMAND cmd = get_instruction(args[0]);

    instruction_handler(cmd, &c, hash_t, args, &cls[i]);
    // Deallocated the args in every loop
    free_args_list(args, c);
    return 0;
}

void handle_poll_events(struct pollfd *pfds, int *pfdscount, struct client *clients, Data **hash_t)
{
    for (int i = 0; i < (*pfdscount); i++)
    {
        if (pfds[i].revents & (POLLIN | POLLHUP | POLLERR))
        {
            if (i == 0)
                handle_new_connection(pfds, pfdscount, clients);
            else
            {
                if (handle_request(pfds, i, pfdscount, clients, hash_t) == 1)
                    i--;
                else
                    send_response(&pfds[i], &clients[i]);
            }
        }
    }
}

struct client *init_clients(int fd, int size)
{
    // SIZE_CLIENTS = size; // Global variable that size of array of clients
    struct client *cls = calloc(sizeof(struct client), size);
    cls[0].fd = fd;
    return cls;
}

struct pollfd *init_poll(int *pfdscount, int sockfd, int size)
{
    struct pollfd *pfds = calloc(sizeof(struct pollfd), size);
    // Add listener file descriptor to poll
    add_to_poll(pfds, pfdscount, sockfd, (POLLIN | POLLHUP | POLLERR), size);
    return pfds;
}
