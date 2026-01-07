#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "ds.h"
#include "hash.h"

Data *create_node(char *key, char *value)
{
    Data *p = (Data *)malloc(sizeof(Data));
    if (p == NULL)
    {
        fprintf(stderr, "Data node could not be created!");
        return NULL;
    }

    p->key = strdup(key);
    p->value = strdup(value);
    p->next = NULL;

    return p;
}

Data **create_hash_table(size_t size)
{
    return calloc(sizeof(Data *), size);
    // return malloc(sizeof(Data *) * size);
}

Data *get_node(Data **table, char *key)
{
    uint8_t index = get_hash_index(key, TABLE_SIZE);
    Data *p = table[index];
    while (p != NULL)
    {
        if (!strcmp(p->key, key))
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

void free_node(Data *node)
{
    free(node->key);
    free(node->value);
    free(node);
}

// Returns true if the node to delete was found, otherwise returns false
bool delete_node(Data **table, char *key)
{
    unsigned int index = get_hash_index(key, TABLE_SIZE);
    // If the linked list is empty, the node to delete is not found
    if (table[index] == NULL)
    {
        return false;
    }
    Data *temp = table[index];
    // Check if the node is the first element to be deleted
    if (!strcmp(table[index]->key, key))
    {
        table[index] = table[index]->next;
        free_node(temp);
        return true;
    }
    // Otherwise, the node is somewhere in the middle of the linked list
    else
    {
        // Travel the linked list
        while (temp->next != NULL)
        {
            // If the node is found, connect the previous and next nodes
            if (!strcmp(temp->next->key, key))
            {
                Data *freed = temp->next;
                temp->next = freed->next;
                free_node(freed);
                return true;
            }
            temp = temp->next;
        }
        return false;
    }
}

void delete_table(Data **table, uint8_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (table[i] != NULL)
        {
            printf("Silinen index <%d>\n", i);
            // First element
            Data *temp = table[i];
            while (temp != NULL)
            {
                Data *freed = temp;
                temp = temp->next;
                free_node(freed);
            }
        }
    }
    free(table);
}

void append_node(Data **table, Data *node)
{
    unsigned int index = get_hash_index(node->key, TABLE_SIZE);
    if (table[index] != NULL)
    {
        fprintf(stdout, "Collision detected, node will append the same index.\n");
        node->next = table[index];
    }
    table[index] = node;
}

void print_node(Data *node)
{
    fprintf(stdout, "[%s : %s]", node->key, node->value);
}

void print_table(Data **table)
{
    for (int i = 0; i < TABLE_SIZE; ++i)
    {
        if (table[i] != NULL)
        {
            fprintf(stdout, "Index <%d>: ", i);
            Data *p = table[i];
            while (p != NULL)
            {
                print_node(p);
                printf(" -> ");
                p = p->next;
            }
            printf("NULL\n");
        }
    }
}