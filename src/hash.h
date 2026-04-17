#ifndef HASH_H
#define HASH_H

unsigned long hash_djb2(unsigned char *str);
unsigned int get_hash_index(char *key, uint16_t size);
#endif