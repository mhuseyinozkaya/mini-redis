
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdlib.h>
#include <signal.h>
#include "structure.h"
#include "parser.h"

extern struct client *cl;
/*
// Function pointer declaration
typedef void (*InstructionPointer)(int argc, char *argv[]);

typedef struct{
    const char* func_name;
    InstructionPointer func;
}Instructions;

// Instruction declaration these are wrapper functions will be called from main()
void cmd_get(int argc, char *argv[]);
void cmd_set(int argc, char *argv[]);
void cmd_del(int argc, char *argv[]);
void cmd_save(int argc, char *argv[]);
void cmd_exit(int argc, char *argv[]);

extern const Instructions command_table[];
*/
void instruction_handler(COMMAND cmd ,int *c, Data **hash_t, char **args,struct client *cl);
void cmd_get(Data **table, char **args, struct client *cl);
void cmd_set(Data **table, char **args, struct client *cl);
void cmd_del(Data **table, char **args, struct client *cl);
void cmd_save(Data** table,unsigned int size);
void cmd_load(Data** table);
void cmd_exit(volatile sig_atomic_t *keep_run);
#endif
