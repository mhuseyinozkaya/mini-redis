#ifndef TEST_H
#define TEST_H

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

int handle_request_v2(struct pollfd *pfds, int i, int *pfdscount, struct client *cl, Data **hash_t);
void handle_poll_events_v2(struct pollfd *pfds, int *pfdscount, struct client *cl, Data **hash_t);
void handle_new_connection_v2(struct pollfd *pfds, int *pfdscount,struct client *clients);
#endif