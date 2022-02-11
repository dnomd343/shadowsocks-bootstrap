#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "common.h"
#include "network.h"
#include "process.h"

int is_udp_proxy;
char *server_addr, *client_addr;
char *server_port, *client_port;
char *password;
char *method;
char *timeout;
int fastopen;
char *plugin;
char *plugin_opts;
char *shadowsocks;
char **shadowsocks_opts;

void args_dump();
void args_init();
void error_exit(char *msg);
char* int_to_string(int num);
void pack_shadowsocks_params();
char* read_file(char *file_name);
void json_decode(char *json_content);
void add_shadowsocks_option(char *option);
void extra_options_decode(char *extra_opts);

void error_exit(char *msg) { // throw error message with exit-code 1
    printf("[Shadowsocks Bootstrap] ERROR: %s.\n", msg);
    printf("[Shadowsocks Bootstrap] exit with error.\n");
    exit(1);
}

char* int_to_string(int num) { // int -> string
    if (num < 0) {
        error_exit("number must be positive");
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

void params_load(char *ss_default) { // load shadowsocks and plugin params
    if (shadowsocks == NULL) {
        shadowsocks = ss_default;
    }
    shadowsocks_opts[0] = shadowsocks; // fill with file name
    if (plugin != NULL) { // with plugin
        char *rand_port = int_to_string(get_available_port(RANDOM_PORT_START, RANDOM_PORT_END));
        SS_REMOTE_HOST = server_addr;
        SS_REMOTE_PORT = server_port;
        SS_LOCAL_HOST = "127.0.0.1";
        SS_LOCAL_PORT = rand_port;
        server_addr = SS_LOCAL_HOST;
        server_port = SS_LOCAL_PORT;
        SS_PLUGIN_OPTIONS = plugin_opts;
    }
    pack_shadowsocks_params();
    shadowsocks_args = shadowsocks_opts;
    if (plugin == NULL) {
        plugin_file = NULL;
    } else {
        plugin_file = plugin;
    }
}

void args_init() { // init arguments
    is_udp_proxy = 1; // udp proxy in default
    server_addr = client_addr = NULL;
    server_port = client_port = NULL;
    password = NULL;
    method = NULL;
    timeout = NULL;
    fastopen = 0;
    plugin = NULL;
    plugin_opts = NULL;
    shadowsocks = NULL;
    shadowsocks_opts = (char**)malloc(sizeof(char*) * 2);
    shadowsocks_opts[0] = ""; // reserved for program name
    shadowsocks_opts[1] = NULL;
}

char* read_file(char *file_name) { // read file content
    FILE *pfile = fopen(file_name, "rb");
    if (pfile == NULL) { // open failed
        char *msg_prefix = "File `";
        char *msg_suffix = "` open failed";
        char *msg = (char*)malloc(strlen(msg_prefix) + strlen(file_name) + strlen(msg_suffix) + 1);
        strcpy(msg, msg_prefix);
        error_exit(strcat(strcat(msg, file_name), msg_suffix)); // merge error message
    }
    fseek(pfile, 0, SEEK_END);
    long file_length = ftell(pfile); // get file length
    char *file_content = (char*)malloc(file_length + 1); // malloc new memory
    if (file_content == NULL) {
        error_exit("no enough memory"); // file too large
    }
    rewind(pfile);
    fread(file_content, 1, file_length, pfile); // read file stream
    file_content[file_length] = '\0'; // set end flag
    fclose(pfile);
    return file_content;
}

void extra_options_decode(char *extra_opts) { // decode shadowsocks extra options
    int num, i;
    char *tmp = (char*)calloc(strlen(extra_opts) + 1, 1); // new memory and set as 0x00
    num = i = 0;
    for (;;) {
        if (extra_opts[num] == '\0' || extra_opts[num] == ' ') { // string end or find a space
            tmp[i] = '\0';
            if (i) { // ignore empty string
                add_shadowsocks_option(tmp);
            }
            if (extra_opts[num] == '\0') { // string end
                break;
            }
            num++;
            i = 0;
            continue;
        }
        if (extra_opts[num] == '\\') { // skip '\' char
            num++;
        }
        tmp[i++] = extra_opts[num++]; // copy char
    }
}

void add_shadowsocks_option(char *option) { // add shadowsocks options
    int opt_num = 0;
    while(shadowsocks_opts[opt_num++] != NULL); // get options number
    shadowsocks_opts = (char**)realloc(shadowsocks_opts, sizeof(char**) * (opt_num + 1));
    shadowsocks_opts[opt_num - 1] = strcpy((char*)malloc(strlen(option) + 1), option);
    shadowsocks_opts[opt_num] = NULL; // end sign
}

void pack_shadowsocks_params() { // packaging shadowsocks parameters
    if (server_addr != NULL) {
        add_shadowsocks_option("-s");
        add_shadowsocks_option(server_addr);
    }
    if (client_addr != NULL) {
        add_shadowsocks_option("-b");
        add_shadowsocks_option(client_addr); 
    }
    if (server_port != NULL) {
        add_shadowsocks_option("-p");
        add_shadowsocks_option(server_port);
    }
    if (client_port != NULL) {
        add_shadowsocks_option("-l");
        add_shadowsocks_option(client_port);
    }
    if (password != NULL) {
        add_shadowsocks_option("-k");
        add_shadowsocks_option(password);
    }
    if (method != NULL) {
        add_shadowsocks_option("-m");
        add_shadowsocks_option(method);
    }
    if (timeout != NULL) {
        add_shadowsocks_option("-t");
        add_shadowsocks_option(timeout);
    }
    if (fastopen) {
        add_shadowsocks_option("--fast-open");
    }
}

void json_decode(char *json_content) { // decode JSON content
    cJSON* json = NULL;
    json = cJSON_Parse(json_content);
    if (json == NULL) {
        error_exit("JSON format error.\n");
    }
    json = json->child;
    while (json != NULL) {
        if (!strcmp(json->string, "no_udp")) { // no_udp => without udp proxy
            if (!cJSON_IsBool(json)) {
                error_exit("`no_udp` must be a bool.\n");
            }
            if (json->valueint) { // is_udp_proxy = ~(json->valueint)
                is_udp_proxy = 0;
            } else {
                is_udp_proxy = 1;
            }
        } else if (!strcmp(json->string, "server")) { // server => server_addr
            if (!cJSON_IsString(json)) {
                error_exit("`server` must be a string.\n");
            }
            if (server_addr != NULL) {
                free(server_addr);
            }
            server_addr = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "local_address")) { // local_address => client_addr
            if (!cJSON_IsString(json)) {
                error_exit("`local_address` must be a string.\n");
            }
            if (client_addr != NULL) {
                free(client_addr);
            }
            client_addr = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "server_port")) { // server_port => server_port
            if (server_port != NULL) {
                free(server_port);
            }
            if (cJSON_IsNumber(json)) {
                server_port = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                server_port = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
            } else {
                error_exit("`server_port` must be a number or string.\n");
            }
        } else if (!strcmp(json->string, "local_port")) { // local_port => client_port
            if (client_port != NULL) {
                free(client_port);
            }
            if (cJSON_IsNumber(json)) {
                client_port = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                client_port = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
            } else {
                error_exit("`local_port` must be a number or string.\n");
            }
        } else if (!strcmp(json->string, "password")) { // password => password
            if (!cJSON_IsString(json)) {
                error_exit("`password` must be a string.\n");
            }
            if (password != NULL) {
                free(password);
            }
            password = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "timeout")) { // timeout => timeout
            if (timeout != NULL) {
                free(timeout);
            }
            if (cJSON_IsNumber(json)) {
                timeout = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                timeout = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
            } else {
                error_exit("`timeout` must be a number or string.\n");
            }
        } else if (!strcmp(json->string, "method")) { // method => method
            if (!cJSON_IsString(json)) {
                error_exit("`method` must be a string.\n");
            }
            if (method != NULL) {
                free(method);
            }
            method = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "fast_open")) { // fast_open => fastopen
            if (!cJSON_IsBool(json)) {
                error_exit("`fast_open` must be a bool.\n");
            }
            fastopen = json->valueint;
        } else if (!strcmp(json->string, "plugin")) { // plugin => plugin
            if (!cJSON_IsString(json)) {
                error_exit("`plugin` must be a string.\n");
            }
            if (plugin != NULL) {
                free(plugin);
            }
            plugin = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "plugin_opts")) { // plugin_opts => plugin_opts
            if (!cJSON_IsString(json)) {
                error_exit("`plugin_opts` must be a string.\n");
            }
            if (plugin_opts != NULL) {
                free(plugin_opts);
            }
            plugin_opts = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "shadowsocks")) { // shadowsocks => shadowsocks
            if (!cJSON_IsString(json)) {
                error_exit("`shadowsocks` must be a string.\n");
            }
            if (shadowsocks != NULL) {
                free(shadowsocks);
            }
            shadowsocks = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "extra_opts")) { // extra_opts => DECODE => shadowsocks_opts
            if (!cJSON_IsString(json)) {
                error_exit("`extra_opts` must be a string.\n");
            }
            extra_options_decode(json->valuestring);
        } else { // unknown field => ERROR
            char *msg_prefix = "Unknown JSON field `";
            char *msg_suffix = "`.\n";
            char *msg = (char*)malloc(strlen(msg_prefix) + strlen(json->string) + strlen(msg_suffix) + 1);
            strcpy(msg, msg_prefix);
            error_exit(strcat(strcat(msg, json->string), msg_suffix));
        }
        json = json->next; // next field
    }
    cJSON_free(json); // free JSON struct
}

void args_decode(int argc, char **argv) { // decode the input parameters
    args_init();
    int i;
    int debug_flag = 0;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--debug")) { // --debug => dump args
            debug_flag = 1;
        } else if (!strcmp(argv[i], "--no-udp")) { // --no-udp => without udp proxy
            is_udp_proxy = 0;
        } else if (!strcmp(argv[i], "-c")) { // -c => CONFIG_JSON
            if (i + 1 == argc) {
                error_exit("`-c` require a parameter");
            }
            json_decode(read_file(argv[++i]));
        } else if (!strcmp(argv[i], "-s")) { // -s => server_addr
            if (i + 1 == argc) {
                error_exit("`-s` require a parameter");
            }
            if (server_addr != NULL) {
                free(server_addr);
            }
            ++i;
            server_addr = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "-p")) { // -p => server_port
            if (i + 1 == argc) {
                error_exit("`-p` require a parameter");
            }
            if (server_port != NULL) {
                free(server_port);
            }
            ++i;
            server_port = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "-b")) { // -b => client_addr
            if (i + 1 == argc) {
                error_exit("`-b` require a parameter");
            }
            if (client_addr != NULL) {
                free(client_addr);
            }
            ++i;
            client_addr = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "-l")) { // -l => client_port
            if (i + 1 == argc) {
                error_exit("`-l` require a parameter");
            }
            if (client_port != NULL) {
                free(client_port);
            }
            ++i;
            client_port = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "-k")) { // -k => password
            if (i + 1 == argc) {
                error_exit("`-k` require a parameter");
            }
            if (password != NULL) {
                free(password);
            }
            ++i;
            password = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "-m")) { // -m => method
            if (i + 1 == argc) {
                error_exit("`-m` require a parameter");
            }
            if (method != NULL) {
                free(method);
            }
            ++i;
            method = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "-t")) { // -t => timeout
            if (i + 1 == argc) {
                error_exit("`-t` require a parameter");
            }
            if (timeout != NULL) {
                free(timeout);
            }
            ++i;
            timeout = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "--fast-open")) { // --fast-open
            fastopen = 1;
        } else if (!strcmp(argv[i], "--plugin")) { // --plugin => plugin
            if (i + 1 == argc) {
                error_exit("`--plugin` require a parameter");
            }
            if (plugin != NULL) {
                free(plugin);
            }
            ++i;
            plugin = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "--plugin-opts")) { // --plugin-opts => plugin_opts
            if (i + 1 == argc) {
                error_exit("`--plugin-opts` require a parameter");
            }
            if (plugin_opts != NULL) {
                free(plugin_opts);
            }
            ++i;
            plugin_opts = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else if (!strcmp(argv[i], "--shadowsocks")) { // --shadowsocks => shadowsocks
            if (i + 1 == argc) {
                error_exit("`--shadowsocks` require a parameter");
            }
            if (shadowsocks != NULL) {
                free(shadowsocks);
            }
            ++i;
            shadowsocks = strcpy((char*)malloc(strlen(argv[i]) + 1), argv[i]);
        } else { // unknown option => archive
            add_shadowsocks_option(argv[i]); // archive unknown options
        }
    }
    if (server_addr == NULL) { // default server address (bind address in server mode)
        server_addr = "127.0.0.1";
    }
    if (debug_flag) { // show args for debug
        args_dump();
    }
}

void args_dump() { // show parameter's content
    if (is_udp_proxy) {
        printf("is_udp_proxy = true\n");
    } else {
        printf("is_udp_proxy = false\n");
    }
    printf("server_addr = %s\n", server_addr);
    printf("client_addr = %s\n", client_addr);
    printf("server_port = %s\n", server_port);
    printf("client_port = %s\n", client_port);
    printf("password = %s\n", password);
    printf("method = %s\n", method);
    printf("timeout = %s\n", timeout);
    if (fastopen) {
        printf("fastopen = true\n");
    } else {
        printf("fastopen = false\n");
    }
    printf("shadowsocks = %s\n", shadowsocks);
    printf("plugin = %s\n", plugin);
    printf("plugin_opts = %s\n", plugin_opts);
    int num = 0;
    printf("options:\n");
    while(shadowsocks_opts[num] != NULL) {
        printf("  '%s'\n", shadowsocks_opts[num]);
        num++;
    }
}
