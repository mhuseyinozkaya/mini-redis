#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <stdbool.h>
#include <stdint.h>

extern struct client *cl;

typedef struct data
{
    char *key;
    char *value;
    struct data *next;
} Data;

#define TABLE_SIZE 127

Data *create_node(char *key, char *value);
bool delete_node(Data **table, char *key);
Data **create_hash_table(size_t size);
void delete_table(Data **table, uint8_t size);
Data *get_node(Data **table, char *key);
void append_node(Data **table, Data *node);
void free_node(Data *node);
void print_node(Data *node,struct client *cl);
void print_table(Data **table);
#endif
