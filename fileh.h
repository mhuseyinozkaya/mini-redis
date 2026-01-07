#include "ds.h"
#ifndef FILEH_H
#define FILEH_H
int read_from_file(char **key, char **value, FILE *fp);
void write_to_file(Data *p, FILE *fp);
#endif