#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

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

void to_upper(char *str){
    for(int i=0; str[i] != '\0';++i){
        if( 'a' <= str[i] && str[i] <= 'z'){
            str[i] -= 0x20;
        }
    }
}

void to_lower(char *str){
    for(int i=0; str[i] != '\0';++i){
        if( 'A' <= str[i] && str[i] <= 'Z'){
            str[i] += 0x20;
        }
    }
}

char **input_tokenizer(char *str, unsigned int *c)
{
    *c = 0;
    char delims[] = " \t\n";
    char **arr = NULL;
    char *token = strtok(str,delims);
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
        token = strtok(NULL,delims);
    }
    return arr;
}
