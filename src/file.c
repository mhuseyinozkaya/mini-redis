#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "structure.h"

void write_to_file(Data *p, FILE *fp)
{
    int len_key = strlen(p->key);
    int len_value = strlen(p->value);
    fwrite(&len_key, sizeof(int), 1, fp);
    fwrite(p->key, sizeof(char), len_key, fp);
    fwrite(&len_value, sizeof(int), 1, fp);
    fwrite(p->value, sizeof(char), len_value, fp);
}

int read_from_file(char **key, char **value, FILE *fp)
{
    int len_key;
    int len_value;
    int ret;
    // Read the key
    ret = fread(&len_key, sizeof(int), 1, fp);
    if(!ret){
        return 1;
    }
    // Allocate memory for store the key
    *key = malloc((len_key + 1) * sizeof(char));
    fread(*key, sizeof(char), len_key, fp);
    (*key)[len_key] = '\0';

    // Read the value
    ret = fread(&len_value, sizeof(int), 1, fp);
    if(!ret){
        free((*key));
        return 1;
    }

    // Allocate memory for store the value
    *value = malloc((len_value + 1) * sizeof(char));
    fread(*value, sizeof(char), len_value, fp);
    (*value)[len_value] = '\0';

    return 0;
}
