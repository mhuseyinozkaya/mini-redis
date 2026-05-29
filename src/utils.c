#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void free_args_list(char **args, int n)
{
    if (args == NULL)
        return;
    for (int i = 0; i < n; ++i)
    {
        free(args[i]);
    }
    free(args);
}

/* Debug functions */
int print_buffer(char *buffer, int rs)
{
    int i;
    printf("[");
    for (i = 0; i < rs + 1; ++i)
    {
        if (buffer[i] == '\r')
            printf("'\\r'");
        else if (buffer[i] == '\n')
            printf("'\\n'");
        else if (buffer[i] == '\0')
            printf("'\\0'");
        else
            printf("'%c'", buffer[i]);
        printf(", ");
    }
    printf("...]\n");
    return i;
}