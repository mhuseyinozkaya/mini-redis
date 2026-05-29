#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>

#include "parser.h"
#include "structure.h"

// How many client waits in queue to be connect to file descriptor named 'listener'
#define BACKLOG 10

// Maximum size of array of client
#define DEFAULT_MAX_CLIENTS 16
#define SIZE_CLIENTS DEFAULT_MAX_CLIENTS

// MAXIMUM buffer size
#define B_SIZE 4096

enum buffer_state
{
    STATE_ARRAY_HEADER,
    STATE_BULK_HEADER,
    STATE_BULK_DATA,
};

struct cmd_args
{
    char **args;
    int arg_count;
};

struct queue
{
    /* Geçerli komutların sırayla çalıştırmak için saklanacak kuyruk veriyapısı */
    struct cmd_args cmds[16];
    /* Komutlar head'in gösterdiği yerden sırayla çalıştırılacak */
    int head;
    int tail;
    int count;
};
struct buffer
{
    char data[B_SIZE];
    int size;
    int pos;
};
struct client
{
    struct sockaddr_storage addr;
    socklen_t addrlen;
    int fd;
    /* Finite State Machine */
    int current_arg;
    int expected_args;
    int expected_len;
    enum buffer_state parse_state;
    /* Finite State Machine */
    struct queue queue_list;
    struct buffer recv_buf;
    struct buffer send_buf;
    /* Bu yapı sonradan kaldırılacak mevcut kodu bozmamak için şuan duracak */
    char send_buffer[B_SIZE];
    int buffer_size;
    /* Bu yapı sonradan kaldırılacak mevcut kodu bozmamak için şuan duracak */
};
typedef enum message_type
{
    ERROR = -1,
    NIL,
    INFO,
    BULK_STRING,
    ARRAY
} MSG_TYPE;

int handle_response_message(struct client *cl, MSG_TYPE message_type, const char *format, ...);
int get_local_addr(struct addrinfo **res, char *port);
int get_listener_socket(struct addrinfo *p);
int bind_lsock(int sock, struct addrinfo *res);

/* This function returns IP address from the given address */
char *getip_addr(struct sockaddr *addr);
/* returns port from the given sockaddr address */
unsigned short get_port(struct sockaddr *addr);

int add_to_poll(struct pollfd *p, int *index, int sock, short event, int max_size);
int remove_from_poll(struct pollfd *pfds, int i, int *pfdscount);

int handle_request(struct pollfd *pfds, int i, int *pfdscount, struct client *cl, Data **hash_t);
void handle_poll_events(struct pollfd *pfds, int *pfdscount, struct client *cl, Data **hash_t);
void handle_new_connection(struct pollfd *pfds, int *pfdscount, struct client *clients);

struct client *init_clients(int fd, int size);
struct pollfd *init_poll(int *pfdscount, int sockfd, int size);

#endif
