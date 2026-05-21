#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "server.h"
#include "instructions.h"
#include "structure.h"
#include "parser.h"
#include "file.h"

extern volatile sig_atomic_t keep_running;

char *get_current_time()
{
    int size = 64;
    char *buffer = calloc(sizeof(char), size);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%d%m%Y%H%M", t);
    return buffer;
}

void cmd_get(Data **table, char **args, struct client *cl)
{
    Data *p = get_node(table, args[1]);
    if (p == NULL)
        handle_response_message(cl,NIL,"");
    else{
        print_node(p,cl);
        printf("\n");
    }
}

void cmd_set(Data **table, char **args, struct client *cl)
{
    // Search the key in table if the key not exist then create
    Data *p = get_node(table, args[1]);
    if (p == NULL)
    {
        Data *node = create_node(args[1], args[2]);
        append_node(table, node);
        handle_response_message(cl, INFO, "OK");
    }
    // If the key already exist then update the key with new value
    else
    {
        handle_response_message(cl, INFO, "Updating with new value for the key: %s", args[1]);
        free(p->value);
        p->value = strdup(args[2]);
    }
}

void cmd_del(Data **table, char **args, struct client *cl)
{
    if (delete_node(table, args[1]))
        handle_response_message(cl,INFO, "Key successfully deleted: %s", args[1]);
    else
        handle_response_message(cl,ERROR, "ERR Key not found, could not be deleted: %s", args[1]);
}
void cmd_save(Data **table, unsigned int size,struct client *cl)
{
    char str[32] = "rdb";
    char *time = get_current_time();
    char *filename = strcat(str, time);
    free(time);
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL){
        fprintf(stderr, "File could not opened in write mode!\n");
        return;
    }
    for (int i = 0; i < size; i++)
    {
        if (table[i] == NULL)
            continue;
        Data *temp = table[i];
        while (temp != NULL)
        {
            write_to_file(temp, fp);
            temp = temp->next;
        }
    }
    fclose(fp);
    if (rename(filename, "backup.rdb") != EXIT_SUCCESS)
    {
        fprintf(stdout, "Backup file could not renamed: <%s>\n",filename);
    }
    handle_response_message(cl,INFO,"OK");
}

void cmd_load(Data **table, struct client *cl)
{
    char *key;
    char *value;
    char filename[] = "backup.rdb";
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not loaded data from file!\n");
        return;
    }
    while (read_from_file(&key, &value, fp) == EXIT_SUCCESS)
    {
        Data *p = create_node(key, value);
        free(key);
        free(value);
        if (p != NULL)
        {
            append_node(table, p);
        }
    }
    handle_response_message(cl,INFO,"OK");
    fclose(fp);
}

void cmd_exit(volatile sig_atomic_t *keep_run)
{
    *keep_run = 0;
}

void instruction_handler(COMMAND cmd ,int *c, Data **hash_t, char **args,struct client *cl){
    switch (cmd)
    {
        case CMD_GET:
            if ((*c - 1) != ARGC_GET)
                handle_response_message(cl,ERROR,"ERR You passed wrong parameters to GET: %d", *c - 1);
            else
                cmd_get(hash_t, args,cl);
            break;
        case CMD_SET:
            if ((*c - 1) != ARGC_SET)
                handle_response_message(cl,ERROR,"ERR You passed wrong parameters to SET: %d", *c - 1);
            else
                cmd_set(hash_t, args,cl);
            break;
        case CMD_DEL:
            if ((*c - 1) != ARGC_DEL)
                handle_response_message(cl,ERROR,"ERR You passed wrong parameters to DEL: %d", *c - 1);
            else
                cmd_del(hash_t, args,cl);
            break;
        case CMD_SAVE:
            cmd_save(hash_t, TABLE_SIZE,cl);
            break;
        case CMD_LOAD:
            cmd_load(hash_t,cl);
            break;
        case CMD_EXIT:
            cmd_exit(&keep_running);
            break;
        case CMD_UNKNOWN:
            handle_response_message(cl,ERROR,"ERR Unknown command: %s", args[0]);
            break;
        default:
            handle_response_message(cl,INFO,"OK");
            break;
    }
}