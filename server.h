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

#define BACKLOG 10
#define B_SIZE 1024
int errno;

struct client{
    char send_buffer[B_SIZE];
    int buffer_size;
};

int handle_response_message(struct client *cl, const char *format,...);
int get_local_addr(struct addrinfo **res, char *port);
int get_listener_socket(struct addrinfo *p);
int bind_lsock(int sock,struct addrinfo *res);
/* This function returns IP address from the given address */
char *getip_addr(struct sockaddr *addr);
unsigned short get_port(struct sockaddr *addr);
int add_to_poll(struct pollfd *p, int *index, int sock, short event, int max_size);
int remove_from_poll(struct pollfd *pfds, int i, int *pfdscount);
void handle_new_connection(struct pollfd *pfds, int i, int *pfdscount);
int handle_request(struct pollfd *pfds, int i, int *pfdscount, char *buffer, int b_size,Data **hash_t,struct client *cl);
void handle_poll_events(struct pollfd *pfds, int *pfdscount, int listener,char *buffer, int b_size,Data **hash_t, struct client *cl);

#endif
