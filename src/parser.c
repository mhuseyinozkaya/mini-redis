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

int _handle_buffer_pos(struct client *cl, int pos)
{
    cl->recv_buf.pos = pos;
    return 0;
}
/* if overflow occurs then returns -1 */
/* if success then returns 0 */
/* if CRLF not checked then 1 */
int check_crlf(char *ptr, int *index, int *offset, int size)
{
    int dummy = 0x0;
    if (offset == NULL)
    {
        offset = &dummy;
    }
    if ((*index + *offset + 1) < size)
    {
        if (ptr[*index + *offset] == '\r' && ptr[*index + *offset + 1] == '\n')
            // Jump next of \r\n pointer index
            *index = (*index + *offset + 2);
        else
            return 1;
    }
    else
        return -1;
    return EXIT_SUCCESS;
}

/* return -1 on error */
/* return 0 on success */
/* return 1 on partial read */

int resp_decoder(struct client *cl)
{
    char *buffer = cl->recv_buf.data;
    int recvs = cl->recv_buf.size;
    int i = cl->recv_buf.pos;
    char *endptr;
    int ret;
    int start; /* her case'in başlangıç pozisyonu */

    DEBUG_LOG("resp_decoder start: i=%d, size=%d, state=%d\n",
              cl->recv_buf.pos, cl->recv_buf.size, cl->parse_state);

    while (i < recvs)
    {
        switch (cl->parse_state)
        {
        case STATE_ARRAY_HEADER:
            if (buffer[i] != '*')
            {
                i++;
                break;
            }
            start = i; /* '*' pozisyonunu kaydet */

            if (i + 1 >= recvs)
            {
                _handle_buffer_pos(cl, start);
                return -1;
            }

            cl->expected_args = strtol(&buffer[i + 1], &endptr, 10);
            i = endptr - buffer;

            ret = check_crlf(buffer, &i, NULL, recvs);
            if (ret != 0)
            {
                _handle_buffer_pos(cl, start);
                return -1;
            }

            cl->current_arg = 0;
            /* daha önce malloc yapılmadıysa yap */
            if (cl->queue_list.cmds[cl->queue_list.tail].args == NULL)
                cl->queue_list.cmds[cl->queue_list.tail].args =
                    malloc(sizeof(char *) * cl->expected_args);
            cl->queue_list.cmds[cl->queue_list.tail].arg_count = cl->expected_args;
            cl->parse_state = STATE_BULK_HEADER;
            break;

        case STATE_BULK_HEADER:
            start = i; /* '$' pozisyonunu kaydet */

            if (buffer[i] != '$')
            {
                _handle_buffer_pos(cl, start);
                return -1;
            }
            if (i + 1 >= recvs)
            {
                _handle_buffer_pos(cl, start);
                return -1;
            }

            cl->expected_len = strtol(&buffer[i + 1], &endptr, 10);
            i = endptr - buffer;

            ret = check_crlf(buffer, &i, NULL, recvs);
            if (ret != 0)
            {
                _handle_buffer_pos(cl, start);
                return -1;
            }

            cl->parse_state = STATE_BULK_DATA;
            break;

        case STATE_BULK_DATA:
            start = i; /* verinin başlangıcını kaydet */

            ret = check_crlf(buffer, &i, &cl->expected_len, recvs);
            if (ret != 0)
            {
                _handle_buffer_pos(cl, start);
                return -1;
            }

            /* check_crlf i'yi expected_len+2 ilerletti, veri start'tan başlıyor */
            cl->queue_list.cmds[cl->queue_list.tail].args[cl->current_arg] =
                strndup(&buffer[start], cl->expected_len);

            cl->current_arg++;
            if (cl->current_arg == cl->expected_args)
            {
                cl->queue_list.tail = (cl->queue_list.tail + 1) % 16;
                cl->queue_list.count++;
                cl->current_arg = 0;
                cl->expected_args = 0;
                cl->parse_state = STATE_ARRAY_HEADER;
            }
            else
            {
                cl->parse_state = STATE_BULK_HEADER;
            }
            break;

        default:
            return -2;
        }
    }

    cl->recv_buf.size = 0;
    cl->recv_buf.pos = 0;
    return 0;
}

int resp_simple(struct client *cl, const char *type, const char *payload)
{
    cl->send_buf.size = snprintf(cl->send_buf.data, sizeof(cl->send_buf.data), "%s%s\r\n", type, payload);
    return 0;
}
