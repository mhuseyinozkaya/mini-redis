#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/tcp.h>

#include "server.h"
#include "structure.h"
#include "instructions.h"
#include "parser.h"
#include "hash.h"

#include "test.h"

// Global variable for the main loop
volatile sig_atomic_t keep_running = 1;

void interrupt_handler(int signal)
{
    (void)signal;
    keep_running = 0;
}

int main(int argc, char *argv[])
{
    //    static uint8_t load_factor[] = {0, TABLE_SIZE};

    // Create hash table to store data
    Data **hash_t = create_hash_table(TABLE_SIZE);
    if (hash_t == NULL)
    {
        fprintf(stderr, "Hash table could not be created!\n");
        exit(1);
    }
    // Handle the signal SIGINT interrupt
    signal(SIGINT, interrupt_handler);

    /* SERVER */

    /* Searches the server's local address to bind a socket*/
    struct addrinfo *res, *p;
    if (get_local_addr(&res, "6379") == EXIT_FAILURE)
    {
        exit(EXIT_FAILURE);
    }
    /* Traverses the res linked list and breaks the loop at first address found */
    for (p = res; p != NULL; p = p->ai_next)
    {
        char *ipstr = getip_addr(p->ai_addr);
        if (ipstr != NULL)
        {
            fprintf(stdout, "The listener socket will be bind the IP address: <%s>\n", ipstr);
            free(ipstr);
            break;
        }
    }
    /* Get a listener file descriptor socket */
    int listener; // A file descriptor
    if ((listener = get_listener_socket(p)) == -1)
    {
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    /* Associate local adress with a listener socket */
    bind_lsock(listener, p);

    /* Free resources of the linked list res */
    freeaddrinfo(res);

    // Prepare to accept connections on socket FD
    if (listen(listener, BACKLOG) == -1)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    int pfdscount = 0;
    int rv;
#ifdef TEST
    struct client *clients = init_clients(listener, DEFAULT_MAX_CLIENTS);
    struct pollfd *pfds = init_poll(&pfdscount, listener, DEFAULT_MAX_CLIENTS);
#else
    // Fix that backlog
    struct pollfd pfds[BACKLOG + 1];
    memset(&pfds, 0, sizeof pfds);
    add_to_poll(pfds, &pfdscount, listener, (POLLIN | POLLHUP | POLLERR), BACKLOG + 1);
/* SERVER END */
    int b_size = 1024;
    char buffer[b_size];
    struct client cl;
#endif
    while (keep_running)
    {
        /* SERVER */
        rv = poll(pfds, pfdscount, -1);
        if (rv == -1)
        {
            perror("poll() error");
        }
#ifdef TEST
        handle_poll_events_v2(pfds,&pfdscount,clients,hash_t);
#else  
        memset(&cl, 0, sizeof cl);
        handle_poll_events(pfds, &pfdscount, listener, buffer, sizeof buffer, hash_t, &cl);
        /* SERVER END */
#endif
    }
    print_table(hash_t);
    delete_table(hash_t, TABLE_SIZE);
#ifdef TEST
    free(clients);
    free(pfds);
#endif
    return 0;
}
