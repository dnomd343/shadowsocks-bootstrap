#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "common.h"

char* new_string(char *str) {
    return strcpy((char*)malloc(strlen(str) + 1), str);
}

char* int_to_string(int num) { // int -> string
    if (num < 0) {
        log_fatal("number must be positive");
    }
    int count = 0;
    int temp = num;
    while (temp != 0) { // check the number of digits
        temp /= 10;
        ++count;
    }
    char *str = (char*)malloc(count + 1);
    sprintf(str, "%d", num);
    return str;
}

char** string_list_append(char **string_list, char *data) {
    int num = 0;
    while(string_list[num++] != NULL); // get string list size
    string_list = (char**)realloc(string_list, sizeof(char**) * (num + 1));
    string_list[num - 1] = new_string(data);
    string_list[num] = NULL; // list end sign
    return string_list;
}

char* string_list_join(char **string_list) { // combine string list -> `str_1` `str_2` `str_3` ...
    char *join_str = (char*)malloc(0);
    for (char **str = string_list; *str != NULL; ++str) {
        join_str = (char*)realloc(join_str, strlen(join_str) + strlen(*str) + 4);
        join_str = strcat(strcat(join_str, "`"), *str);
        join_str = strcat(join_str, "` ");
    }
    return join_str;
}

char* read_file(char *file_name) { // read file content
    log_debug("Start read file -> %s", file_name);
    FILE *pfile = fopen(file_name, "rb");
    if (pfile == NULL) { // file open failed
        log_fatal("File `%s` open failed", file_name);
    }
    fseek(pfile, 0, SEEK_END);
    long file_length = ftell(pfile); // get file length
    char *file_content = (char*)malloc(file_length + 1); // malloc new memory
    if (file_content == NULL) {
        log_fatal("No enough memory for reading file"); // file too large
    }
    rewind(pfile);
    fread(file_content, 1, file_length, pfile); // read file stream
    file_content[file_length] = '\0'; // set end flag
    fclose(pfile);
    log_debug("File `%s` read success ->\n%s", file_name, file_content);
    return file_content;
}

void init(int argc, char **argv, char *help_msg) {
    if (argc <= 1) { // with only one argument
        printf("%s", help_msg);
        exit(0);
    }
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "--debug")) { // include `--debug`
            LOG_LEVEL = LOG_DEBUG;
        }
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { // include `-h` or `--help`
            printf("%s", help_msg);
            exit(0);
        }
    }
}
