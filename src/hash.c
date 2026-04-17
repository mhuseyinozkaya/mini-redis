#include <stdint.h>

#include "hash.h"

unsigned long hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

unsigned int get_hash_index(char *key, uint16_t size){
    unsigned long hash = hash_djb2(key);
    return hash % size;
}