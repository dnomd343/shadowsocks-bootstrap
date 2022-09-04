#ifndef _COMMON_H_
#define _COMMON_H_

#define VERSION "0.9.2"

#define RANDOM_PORT_START 41952
#define RANDOM_PORT_END   65535

char* new_string(char *str);
char* int_to_string(int num);
char* read_file(char *file_name);
char** string_list_append(char **string_list, char *data);

void init(int argc, char **argv, char *help_msg);

#endif
