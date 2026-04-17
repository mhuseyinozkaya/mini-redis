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
#include "test.h"
#include "utils.h"

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

void handle_new_connection_v2(struct pollfd *pfds, int *pfdscount, struct client *clients)
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

    if (add_new_client(&clients[(*pfdscount)-1], clientfd, client_addr, client_addrlen) != EXIT_SUCCESS)
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

int handle_response_message_v2(struct client *cl, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(cl->send_buffer, B_SIZE, format, args);
    va_end(args);

    cl->buffer_size = strlen(cl->send_buffer);
    fprintf(stdout, "[Log]: message: %s, size: %d\n", cl->send_buffer, cl->buffer_size);
    return 0;
}

int send_response_v2(struct pollfd *pfds, struct client *cl)
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
int handle_request_v2(struct pollfd *pfds, int i, int *pfdscount, struct client *cls, Data **hash_t)
{
    cls[i].recieved_size = recv(pfds[i].fd, cls[i].recv_buf, B_SIZE, 0);
    cls[i].recv_buf[cls[i].recieved_size] = '\0';
#ifdef DBG
    printf("recieved_size: %d\n",cls[i].recieved_size);
    debug_buffer(cls[i].recv_buf,cls[i].recieved_size);
#endif
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
#ifdef REDIS
    char **args = redis_tokenizer(cls[i].recv_buf, cls[i].recieved_size,&c);
#else
    char **args = input_tokenizer(cls[i].recv_buf, &c);
#endif
    if (args == NULL)
        return -1;
#ifdef DBG
printf("\nargs count: %d\n",c);
    for (int i = 0; i < c; i++)
    {
        printf("args[%d]: ",i);
        debug_buffer(args[i],100);
        printf("\n");
        fflush(stdout);
    }
#endif
    // Convert upper the instruction name
    to_upper(args[0]);
    // Get instruction and perform actions
    COMMAND cmd = get_instruction(args[0]);

    instruction_handler(cmd, &c, hash_t, args, &cls[i]);
    // Deallocated the args in every loop
    free_args_list(args,c);
    return 0;
}

void handle_poll_events_v2(struct pollfd *pfds, int *pfdscount, struct client *clients, Data **hash_t)
{
    for (int i = 0; i < (*pfdscount); i++)
    {
        if (pfds[i].revents & (POLLIN | POLLHUP | POLLERR))
        {
            if (i == 0)
                handle_new_connection_v2(pfds, pfdscount, clients);
            else
            {
                if (handle_request_v2(pfds, i, pfdscount, clients, hash_t) == 1)
                    i--;
                else
                    send_response_v2(&pfds[i], &clients[i]);
            }
        }
    }
}
