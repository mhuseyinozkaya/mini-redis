#ifndef PARSER_H
#define PARSER_H

enum instructions_argument_count
{
    ARGC_GET = 1,
    ARGC_SET = 2,
    ARGC_DEL = 1,
    ARGC_EXIT = 0
};

typedef enum command
{
    CMD_UNKNOWN,
    CMD_GET,
    CMD_SET,
    CMD_DEL,
    CMD_SAVE,
    CMD_LOAD,
    CMD_EXIT
} COMMAND;


void to_upper(char *str);
void to_lower(char *str);

COMMAND get_instruction(char *str);
char **input_tokenizer(char *str, unsigned int *c);

#endif
