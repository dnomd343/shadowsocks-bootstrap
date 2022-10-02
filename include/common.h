#ifndef COMMON_H_
#define COMMON_H_

#define VERSION "1.0.1-beta"

#define RANDOM_PORT_START 41952
#define RANDOM_PORT_END   65535

void init(int argc, char **argv, const char *help_msg);

char* int_to_string(int num);
char* new_string(const char *str);
char* read_file(const char *file_name);
char* string_list_join(char **string_list);
char** string_list_append(char **string_list, const char *data);

#endif
