#ifndef UTILS_H
#define UTILS_H
#endif

#ifdef DEBUG
#define DEBUG_BUFFER(buffer, size) print_buffer(buffer, size)
#define DEBUG_SILENT(fmt, ...)               \
    do                                       \
    {                                        \
        fprintf(stdout, fmt, ##__VA_ARGS__); \
        fflush(stdout);                      \
    } while (0)
#define DEBUG_LOG(fmt, ...)                             \
    do                                                  \
    {                                                   \
        fprintf(stdout, "[DEBUG] " fmt, ##__VA_ARGS__); \
        fflush(stdout);                                 \
    } while (0)
#else
#define DEBUG_BUFFER(buffer, size)
#define DEBUG_SILENT(fmt, ...)
#define DEBUG_LOG(fmt, ...)
#endif

void free_partial_cmd(struct client *cl);
void free_args_list(char **args, int n);
int print_buffer(char *buffer, int rs);