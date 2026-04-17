#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "server.h"

COMMAND get_instruction(char *str)
{
    if (!strcmp(str, "EXIT"))
        return CMD_EXIT;
    else if (!strcmp(str, "GET"))
        return CMD_GET;
    else if (!strcmp(str, "SET"))
        return CMD_SET;
    else if (!strcmp(str, "DEL"))
        return CMD_DEL;
    else if (!strcmp(str, "SAVE"))
        return CMD_SAVE;
    else if (!strcmp(str, "LOAD"))
        return CMD_LOAD;
    else
        return CMD_UNKNOWN;
}

void to_upper(char *str)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        if ('a' <= str[i] && str[i] <= 'z')
        {
            str[i] -= 0x20;
        }
    }
}

void to_lower(char *str)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        if ('A' <= str[i] && str[i] <= 'Z')
        {
            str[i] += 0x20;
        }
    }
}

char **input_tokenizer(char *str, int *c)
{
    *c = 0;
    char delims[] = " \t\n";
    char **arr = NULL;
    char *token = strtok(str, delims);
    while (token)
    {
        ++(*c);
        char **p = realloc(arr, sizeof(char *) * (*c));
        if (p == NULL)
        {
            fprintf(stderr, "Could not be reallocated!");
            free(arr);
            exit(1);
        }
        arr = p;
        arr[(*c) - 1] = token;
        token = strtok(NULL, delims);
    }
    return arr;
}

char **redis_tokenizer(char *buffer, int recvs, int *c)
{
    int i;
    int number;
    char *endptr;
    char **args = NULL;
    for (i = 0; i < recvs; ++i)
    {
        if(buffer[i] == '*'){
            number = strtol(&buffer[i+1],&endptr,10);
            *c = number; /* argcount */
            i = endptr - buffer;
            if(buffer[i] == '\r' && buffer[i+1] == '\n'){
                args = malloc(sizeof(char*) * (*c));
                printf("\nSuccessfully allocated with argcount: %d\n",*c);
                i+=2;
                break;
            }
        }
    }
    for(int j=0; j < *c; ++j){
        if(buffer[i] == '$'){
            number = strtol(&buffer[i+1],&endptr,10);
            i = endptr - buffer;
            if(buffer[i] == '\r' && buffer[i+1] == '\n'){
                i+=2;
            }

            if(buffer[i+number] == '\r' && buffer[i+number+1] == '\n'){
                args[j] = strndup(&buffer[i],number);
                i+=(number+2);
            }
        }
    }
    return args;
}

int resp_simple(struct client *cl,const char* type, const char *payload){
    cl->buffer_size = snprintf(cl->send_buffer,sizeof(cl->send_buffer),"%s%s\r\n",type,payload);
    return 0;
}