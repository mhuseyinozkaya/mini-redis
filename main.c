#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>

#include "server.h"
#include "structure.h"
#include "instructions.h"
#include "parser.h"
#include "hash.h"

// Global variable for the main loop
volatile sig_atomic_t keep_running = 1;

void interrupt_handler(int signal)
{
    (void)signal;
    keep_running = 0;
}

/*void instruction_handler(COMMAND cmd ,int *c, Data **hash_t, char **args){
    switch (cmd)
    {
        case CMD_GET:
            if ((*c - 1) != ARGC_GET)
                fprintf(stderr, "You passed wrong parameters to GET <%d>\n", *c - 1);
            else
                cmd_get(hash_t, args);
            break;
        case CMD_SET:
            if ((*c - 1) != ARGC_SET)
                fprintf(stderr, "You passed wrong parameters to SET <%d>\n", *c - 1);
            else
                cmd_set(hash_t, args);
            break;
        case CMD_DEL:
            if ((*c - 1) != ARGC_DEL)
                fprintf(stderr, "You passed wrong parameters to DEL <%d>\n", *c - 1);
            else
                cmd_del(hash_t, args);
            break;
        case CMD_SAVE:
            cmd_save(hash_t, TABLE_SIZE);
            break;
        case CMD_LOAD:
            cmd_load(hash_t);
            break;
        case CMD_EXIT:
            cmd_exit(&keep_running);
            break;
        case CMD_UNKNOWN:
            fprintf(stderr, "Unknown command for <%s>\n", args[0]);
            break;
        default:
            break;
    }
}*/

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

    struct addrinfo *res, *p;
    if(get_local_addr(&res,"4444") == EXIT_FAILURE)
    {
        exit(EXIT_FAILURE);
    }
    // Traversing linked list
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
    // Listener socket
    int listener;
    if((listener = get_listener_socket(p)) == -1)
    {
        exit(EXIT_FAILURE);
    }

    bind_lsock(listener,p);

    freeaddrinfo(res);

    // Wait the clients to be conntected
    if (listen(listener, BACKLOG) == -1)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    struct pollfd pfds[BACKLOG + 1];
    memset(&pfds, 0, sizeof pfds);

    int pfdscount = 0;
    int rv;
    // Add our listener to poll
    add_to_poll(pfds, &pfdscount, listener, (POLLIN | POLLHUP | POLLERR), BACKLOG + 1);

    /* SERVER END */

    int b_size = 1024;
    char buffer[b_size];
    struct client cl;
    // int c;
    // char **args = NULL;
    while (keep_running)
    {
        /* SERVER */
        rv = poll(pfds, pfdscount, -1);
        if (rv == -1)
        {
            perror("poll() error");
        }
        memset(&cl,0,sizeof cl);
        handle_poll_events(pfds,&pfdscount,listener,buffer,sizeof buffer,hash_t,&cl);
        /* SERVER END */

        /*
        fprintf(stdout, "MiniRedis-CLI> ");
        if (fgets(buffer, b_size, stdin))
        {
            args = input_tokenizer(buffer, &c);

            if (args == NULL)
                continue;
            // Convert upper the instruction name
            to_upper(args[0]);

            // Get instruction and perform actions
            COMMAND command = get_instruction(args[0]);

            instruction_handler(command,&c,hash_t,args);
            // Deallocated the args in every loop
            free(args);
        }*/
    }
    print_table(hash_t);
    delete_table(hash_t, TABLE_SIZE);
    return 0;
}
