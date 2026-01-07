#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "ds.h"
#include "fileh.h"
#include "instructions.h"

/*
   void cmd_get(int argc, char *argv[]){

   }
   void cmd_set(int argc, char *argv[]){

   }
   void cmd_del(int argc, char *argv[]){

   }
   */

char *get_current_time()
{
    int size = 64;
    char *buffer = calloc(sizeof(char), size);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%d.%m.%Y.%H.%M", t);
    return buffer;
}
void cmd_save(Data **table, unsigned int size)
{
    char str[32] = "rdb";
    char *time = get_current_time();
    char *filename = strcat(str, time);
    free(time);
    FILE *fp = fopen(filename, "wb");
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
    if (!rename(filename, "backup.rdb"))
    {
        fprintf(stdout, "File name was changed.\n");
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
        fprintf(stderr, "File can not opened!");
        return;
    }
    while (read_from_file(&key, &value, fp) == 0)
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

void cmd_exit(int argc, char *argv[])
{
}

/*const Instructions command_table[] = {
  {"GET",cmd_get},
  {"SET",cmd_set},
  {"DEL",cmd_del},
  {"SAVE",cmd_save},
  {"EXIT",cmd_exit},
  {NULL,NULL}
};*/
