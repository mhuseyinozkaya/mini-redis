#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "utils.h"
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

int _handle_buffer_pos(struct client *cl, int pos){
    cl->recv_buf.pos = pos;
    return 0;
} 

/* return -1 on error */
/* return 0 on success */
/* return 1 on partial read */

int resp_decoder(struct client *cl)
{
    char *buffer = cl->recv_buf.data;
    int recvs = cl->recv_buf.size;
    int i = 0;
    int num = 0;      // array eleman sayısı (*N)
    int arg_len = 0;  // bulk string uzunluğu ($N)
    char *endptr;
    while (i < recvs)
    {
        // find RESP array length
        if (buffer[i] == '*')
        {
            if (i + 1 < recvs)
                num = strtol(&buffer[i + 1], &endptr, 10);
            else{
                _handle_buffer_pos(cl,i+1);
                return 1;
            }
            i = endptr - buffer; // mutlak offset
            if (i == 0)
                return -1;
            // memory allocate for args and set arg_count
            if (i + 1 < recvs){
                if (buffer[i] == '\r' && buffer[i + 1] == '\n')
                {
                    cl->queue_list.cmds[cl->queue_list.tail].arg_count = num;
                    cl->queue_list.cmds[cl->queue_list.tail].args = malloc(sizeof(char *) * num);
                    i += 2; // \r\n atla
                }
            }else{
                _handle_buffer_pos(cl,i+1);
                return -1;
            }
            // pass buffer to args
            for (int j = 0; j < num; j++)
            {
                if (i < recvs){
                    if (buffer[i] == '$')
                    {
                        arg_len = strtol(&buffer[i + 1], &endptr, 10);
                        i = endptr - buffer;

                        if ((i + 1) < recvs && buffer[i] == '\r' && buffer[i + 1] == '\n')
                            i += 2;
                        else{
                            _handle_buffer_pos(cl,i+1);
                            return 1;
                        }
                    }
                }else{
                    _handle_buffer_pos(cl,i+1);
                    return 1;
                }

                if((i + arg_len + 1) < recvs){
                    if (buffer[i + arg_len] == '\r' && buffer[i + arg_len + 1] == '\n')
                    {
                        cl->queue_list.cmds[cl->queue_list.tail].args[j] = strndup(&buffer[i], arg_len);
                        i += (arg_len + 2);
                    }
                }else{
                    int k = 0;
                    while(i+k < recvs) k++;
                    _handle_buffer_pos(cl,i+k);
                    return 1;
                }
            }
            // add commands to queue list to execute
            cl->queue_list.tail = (cl->queue_list.tail + 1) % 16;
            cl->queue_list.count++;
        }
        else
        {
            i++;
        }
    }
    return 0;
}

int resp_simple(struct client *cl, const char *type, const char *payload)
{
    cl->buffer_size = snprintf(cl->send_buffer, sizeof(cl->send_buffer), "%s%s\r\n", type, payload);
    return 0;
}