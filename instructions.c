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
        //        fprintf(stderr, "The key not found: %s\n", args[1]);
        handle_response_message(cl,"The key not found: %s\n", args[1]);
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
    }
    // If the key already exist then update the key with new value
    else
    {
        //        fprintf(stdout, "Updating with new value for the key: %s\n", args[1]);
        handle_response_message(cl, "Updating with new value for the key: %s\n", args[1]);
        free(p->value);
        p->value = strdup(args[2]);
    }
}

void cmd_del(Data **table, char **args, struct client *cl)
{
    if (delete_node(table, args[1]))
        //        fprintf(stdout, "Key successfully deleted: %s\n", args[1]);
        handle_response_message(cl, "Key successfully deleted: %s\n", args[1]);
    else
        //        fprintf(stderr, "Key not found, could not be deleted: %s\n", args[1]);
        handle_response_message(cl, "Key not found, could not be deleted: %s\n", args[1]);
}
void cmd_save(Data **table, unsigned int size)
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
}

void cmd_load(Data **table)
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
                //                fprintf(stderr, "You passed wrong parameters to GET <%d>\n", *c - 1);
                handle_response_message(cl,"You passed wrong parameters to GET <%d>\n", *c - 1);
            else
                cmd_get(hash_t, args,cl);
            break;
        case CMD_SET:
            if ((*c - 1) != ARGC_SET)
                //                fprintf(stderr, "You passed wrong parameters to SET <%d>\n", *c - 1);
                handle_response_message(cl,"You passed wrong parameters to SET <%d>\n", *c - 1);
            else
                cmd_set(hash_t, args,cl);
            break;
        case CMD_DEL:
            if ((*c - 1) != ARGC_DEL)
                //                fprintf(stderr, "You passed wrong parameters to DEL <%d>\n", *c - 1);
                handle_response_message(cl,"You passed wrong parameters to DEL <%d>\n", *c - 1);
            else
                cmd_del(hash_t, args,cl);
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
            //            fprintf(stderr, "Unknown command for <%s>\n", args[0]);
            handle_response_message(cl,"Unknown command for <%s>\n", args[0]);
            break;
        default:
            /**/
            break;
    }
}
/*const Instructions command_table[] = {
  {"GET",cmd_get},
  {"SET",cmd_set},
  {"DEL",cmd_del},
  {"SAVE",cmd_save},
  {"EXIT",cmd_exit},
  {NULL,NULL}
  };
  void cmd_get(int argc, char *argv[]){

  }
  void cmd_set(int argc, char *argv[]){

  }
  void cmd_del(int argc, char *argv[]){

  }
  */
