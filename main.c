#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>

#include "ds.h"
#include "instructions.h"
#include "parser.h"
#include "hash.h"

volatile sig_atomic_t keep_running = 1;

void interrupt_handler(int signal)
{
    (void)signal;
    keep_running = 0;
}

int main(int argc, char *argv[])
{
    static uint8_t load_factor[] = {0, TABLE_SIZE};
    Data **hash_t = create_hash_table(TABLE_SIZE);
    if (hash_t == NULL)
    {
        fprintf(stderr, "Hash table could not be created!\n");
        exit(1);
    }
    signal(SIGINT, interrupt_handler);
    unsigned int c;
    char buffer[256];
    char **args = NULL;
    while (keep_running)
    {
        fprintf(stdout, "MiniRedis-CLI> ");
        if (fgets(buffer, 256, stdin))
        {
            args = input_tokenizer(buffer, &c);

            if (args == NULL)
                continue;

            to_upper(args[0]);

            COMMAND command = get_instruction(args[0]);
            switch (command)
            {
            case CMD_GET:
                if ((c - 1) != ARGC_GET)
                    fprintf(stderr, "You passed wrong parameters to GET <%d>\n", c - 1);
                else
                {
                    Data *p = get_node(hash_t, args[1]);
                    if (p == NULL)
                        fprintf(stderr, "The key not found in database: %s\n", args[1]);
                    else
                        print_node(p);
                }
                break;
            case CMD_SET:
                if ((c - 1) != ARGC_SET)
                {
                    fprintf(stderr, "You passed wrong parameters to SET <%d>\n", c - 1);
                }
                else
                {
                    Data *p = get_node(hash_t, args[1]);
                    if (p == NULL)
                    {
                        Data *node = create_node(args[1], args[2]);
                        append_node(hash_t, node);
                    }
                    else
                    {
                        fprintf(stdout, "UPDATING value of the key: %s\n", args[1]);
                        free(p->value);
                        p->value = strdup(args[2]);
                    }
                }
                break;
            case CMD_DEL:
                if ((c - 1) != ARGC_DEL)
                    fprintf(stderr, "You passed wrong parameters to DEL <%d>\n", c - 1);
                else
                {
                    bool is_deleted = delete_node(hash_t, args[1]);
                    if (is_deleted)
                        fprintf(stderr, "Key successfully deleted: %s", args[1]);
                    else
                        fprintf(stderr, "Key not found, could not be deleted: %s", args[1]);
                }
                break;
            case CMD_EXIT:
                keep_running = 0;
                break;
            case CMD_SAVE:
                cmd_save(hash_t, TABLE_SIZE);
                break;
            case CMD_LOAD:
                cmd_load(hash_t);
                break;
            case CMD_UNKNOWN:
                fprintf(stderr, "Unknown command for <%s>\n", args[0]);
                break;
            default:
                /**/
                break;
            }
            free(args);
        }
    }
    print_table(hash_t);
    delete_table(hash_t, TABLE_SIZE);
    return 0;
}
