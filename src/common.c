#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "common.h"
#include "network.h"
#include "process.h"
#include "logger.h"

//int is_udp_proxy = 1;
//char *server_addr = NULL, *client_addr = NULL;
//char *server_port = NULL, *client_port = NULL;
//char *password = NULL;
//char *method = NULL;
//char *timeout = NULL;
//int fastopen = 0;
//char *plugin = NULL;
//char *plugin_opts = NULL;
//char *shadowsocks = NULL;
//char **shadowsocks_opts; // init before usage

void args_dump();
//void error_exit(char *msg);
char* int_to_string(int num);
void pack_shadowsocks_params(bootstrap_info *info);
char* read_file(char *file_name);
//void json_decode(char *json_content);
void add_shadowsocks_option(char *option, char **opts);
void extra_options_decode(char *extra_opts, char **opts);
//void params_load(char *ss_default, bootstrap_info *info);

//void error_exit(char *msg) { // throw error message with exit-code 1
//    printf("[Shadowsocks Bootstrap] ERROR: %s.\n", msg);
//    printf("[Shadowsocks Bootstrap] exit with error.\n");
//    exit(1);
//}

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

void params_load(char *ss_default, bootstrap_info *info) { // load shadowsocks and plugin params
    if (info->shadowsocks == NULL) {
        info->shadowsocks = ss_default;
    }
    info->shadowsocks_opts[0] = info->shadowsocks; // fill with file name
    if (info->plugin != NULL) { // with plugin
        char *rand_port = int_to_string(get_available_port(RANDOM_PORT_START, RANDOM_PORT_END));
        SS_REMOTE_HOST = info->server_addr;
        SS_REMOTE_PORT = info->server_port;
        SS_LOCAL_HOST = "127.0.0.1";
        SS_LOCAL_PORT = rand_port;
        info->server_addr = SS_LOCAL_HOST;
        info->server_port = SS_LOCAL_PORT;
        SS_PLUGIN_OPTIONS = info->plugin_opts;
    }
    pack_shadowsocks_params(info);
    shadowsocks_args = info->shadowsocks_opts;
    if (info->plugin == NULL) {
        plugin_file = NULL;
    } else {
        plugin_file = info->plugin;
    }
}

//void args_init() { // init bootstrap arguments
//    is_udp_proxy = 1; // enable udp proxy in default
//    server_addr = client_addr = NULL;
//    server_port = client_port = NULL;
//    password = NULL;
//    method = NULL;
//    timeout = NULL;
//    fastopen = 0;
//    plugin = NULL;
//    plugin_opts = NULL;
//    shadowsocks = NULL;
//    shadowsocks_opts = (char**)malloc(sizeof(char*) * 2);
//    shadowsocks_opts[0] = ""; // reserved for program name
//    shadowsocks_opts[1] = NULL;
//}

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



void add_shadowsocks_option(char *option, char **opts) { // add shadowsocks options
    int opt_num = 0;
    while(opts[opt_num++] != NULL); // get options number
    opts = (char**)realloc(opts, sizeof(char**) * (opt_num + 1));
    opts[opt_num - 1] = strcpy((char*)malloc(strlen(option) + 1), option);
    opts[opt_num] = NULL; // end sign
}

void pack_shadowsocks_params(bootstrap_info *info) { // packaging shadowsocks parameters
    if (info->server_addr != NULL) {
        add_shadowsocks_option("-s", info->shadowsocks_opts);
        add_shadowsocks_option(info->server_addr, info->shadowsocks_opts);
    }
    if (info->client_addr != NULL) {
        add_shadowsocks_option("-b", info->shadowsocks_opts);
        add_shadowsocks_option(info->client_addr, info->shadowsocks_opts);
    }
    if (info->server_port != NULL) {
        add_shadowsocks_option("-p", info->shadowsocks_opts);
        add_shadowsocks_option(info->server_port, info->shadowsocks_opts);
    }
    if (info->client_port != NULL) {
        add_shadowsocks_option("-l", info->shadowsocks_opts);
        add_shadowsocks_option(info->client_port, info->shadowsocks_opts);
    }
    if (info->password != NULL) {
        add_shadowsocks_option("-k", info->shadowsocks_opts);
        add_shadowsocks_option(info->password, info->shadowsocks_opts);
    }
    if (info->method != NULL) {
        add_shadowsocks_option("-m", info->shadowsocks_opts);
        add_shadowsocks_option(info->method, info->shadowsocks_opts);
    }
    if (info->timeout != NULL) {
        add_shadowsocks_option("-t", info->shadowsocks_opts);
        add_shadowsocks_option(info->timeout, info->shadowsocks_opts);
    }
    if (info->fastopen) {
        add_shadowsocks_option("--fast-open", info->shadowsocks_opts);
    }
}



char* new_string(char *str) {
    return strcpy((char*)malloc(strlen(str) + 1), str);
}

void args_debug(bootstrap_info *info) {
    if (info->is_udp_proxy) {
        log_debug("is_udp_proxy = true");
    } else {
        log_debug("is_udp_proxy = false");
    }
    printf("server_addr = %s\n", info->server_addr);
    printf("client_addr = %s\n", info->client_addr);
    printf("server_port = %s\n", info->server_port);
    printf("client_port = %s\n", info->client_port);
    printf("password = %s\n", info->password);
    printf("method = %s\n", info->method);
    printf("timeout = %s\n", info->timeout);
    if (info->fastopen) {
        printf("fastopen = true\n");
    } else {
        printf("fastopen = false\n");
    }
    printf("shadowsocks = %s\n", info->shadowsocks);
    printf("plugin = %s\n", info->plugin);
    printf("plugin_opts = %s\n", info->plugin_opts);
    int num = 0;
    printf("options:\n");
    while(info->shadowsocks_opts[num] != NULL) {
        printf("  '%s'\n", info->shadowsocks_opts[num]);
        num++;
    }
}


//void args_dump() { // show parameter's content
//
//}
