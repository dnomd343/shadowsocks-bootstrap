#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "common.h"
#include "network.h"
#include "process.h"
#include "log.h"

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
    log_debug("Read file content -> %s", file_name);
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

void extra_options_decode(char *extra_opts, char **opts) { // decode shadowsocks extra options
    int num, i;
    char *tmp = (char*)calloc(strlen(extra_opts) + 1, 1); // new memory and set as 0x00
    num = i = 0;
    for (;;) {
        if (extra_opts[num] == '\0' || extra_opts[num] == ' ') { // string end or find a space
            tmp[i] = '\0';
            if (i) { // ignore empty string
                add_shadowsocks_option(tmp, opts);
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

void json_decode(char *json_content, bootstrap_info *info) { // decode JSON content
    cJSON* json = NULL;
    json = cJSON_Parse(json_content);
    if (json == NULL) {
        log_fatal("JSON format error.");
    }
    json = json->child;
    while (json != NULL) {
        if (!strcmp(json->string, "no_udp")) { // no_udp => without udp proxy
            if (!cJSON_IsBool(json)) {
                log_fatal("`no_udp` must be a bool.");
            }
            if (json->valueint) { // is_udp_proxy = ~(json->valueint)
                info->is_udp_proxy = 0;
            } else {
                info->is_udp_proxy = 1;
            }
        } else if (!strcmp(json->string, "server")) { // server => server_addr
            if (!cJSON_IsString(json)) {
                log_fatal("`server` must be a string.");
            }
            if (info->server_addr != NULL) {
                free(info->server_addr);
            }
            info->server_addr = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "local_address")) { // local_address => client_addr
            if (!cJSON_IsString(json)) {
                log_fatal("`local_address` must be a string.");
            }
            if (info->client_addr != NULL) {
                free(info->client_addr);
            }
            info->client_addr = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "server_port")) { // server_port => server_port
            if (info->server_port != NULL) {
                free(info->server_port);
            }
            if (cJSON_IsNumber(json)) {
                info->server_port = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                info->server_port = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
            } else {
                log_fatal("`server_port` must be a number or string.");
            }
        } else if (!strcmp(json->string, "local_port")) { // local_port => client_port
            if (info->client_port != NULL) {
                free(info->client_port);
            }
            if (cJSON_IsNumber(json)) {
                info->client_port = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                info->client_port = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
            } else {
                log_fatal("`local_port` must be a number or string.");
            }
        } else if (!strcmp(json->string, "password")) { // password => password
            if (!cJSON_IsString(json)) {
                log_fatal("`password` must be a string.");
            }
            if (info->password != NULL) {
                free(info->password);
            }
            info->password = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "timeout")) { // timeout => timeout
            if (info->timeout != NULL) {
                free(info->timeout);
            }
            if (cJSON_IsNumber(json)) {
                info->timeout = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                info->timeout = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
            } else {
                log_fatal("`timeout` must be a number or string.");
            }
        } else if (!strcmp(json->string, "method")) { // method => method
            if (!cJSON_IsString(json)) {
                log_fatal("`method` must be a string.");
            }
            if (info->method != NULL) {
                free(info->method);
            }
            info->method = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "fast_open")) { // fast_open => fastopen
            if (!cJSON_IsBool(json)) {
                log_fatal("`fast_open` must be a bool.");
            }
            info->fastopen = json->valueint;
        } else if (!strcmp(json->string, "plugin")) { // plugin => plugin
            if (!cJSON_IsString(json)) {
                log_fatal("`plugin` must be a string.");
            }
            if (info->plugin != NULL) {
                free(info->plugin);
            }
            info->plugin = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "plugin_opts")) { // plugin_opts => plugin_opts
            if (!cJSON_IsString(json)) {
                log_fatal("`plugin_opts` must be a string.");
            }
            if (info->plugin_opts != NULL) {
                free(info->plugin_opts);
            }
            info->plugin_opts = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "shadowsocks")) { // shadowsocks => shadowsocks
            if (!cJSON_IsString(json)) {
                log_fatal("`shadowsocks` must be a string.");
            }
            if (info->shadowsocks != NULL) {
                free(info->shadowsocks);
            }
            info->shadowsocks = strcpy((char*)malloc(strlen(json->valuestring) + 1), json->valuestring);
        } else if (!strcmp(json->string, "extra_opts")) { // extra_opts => DECODE => shadowsocks_opts
            if (!cJSON_IsString(json)) {
                log_fatal("`extra_opts` must be a string.");
            }
            extra_options_decode(json->valuestring, info->shadowsocks_opts);
        } else { // unknown field => ERROR
            char *msg_prefix = "Unknown JSON field `";
            char *msg_suffix = "`.\n";
            char *msg = (char*)malloc(strlen(msg_prefix) + strlen(json->string) + strlen(msg_suffix) + 1);
            strcpy(msg, msg_prefix);
            log_fatal(strcat(strcat(msg, json->string), msg_suffix));
        }
        json = json->next; // next field
    }
    cJSON_free(json); // free JSON struct
}

void args_init(bootstrap_info *info) {
    info->is_debug = 0; // disable debug mode
    info->is_udp_proxy = 1; // enable udp proxy
    info->server_addr = info->client_addr = NULL;
    info->server_port = info->client_port = NULL;
    info->password = NULL;
    info->method = NULL;
    info->timeout = NULL;
    info->fastopen = 0;
    info->plugin = NULL;
    info->plugin_opts = NULL;
    info->shadowsocks = NULL;
    info->shadowsocks_opts = (char**)malloc(sizeof(char*) * 2); // 2 arguments
    info->shadowsocks_opts[0] = ""; // reserved for program name
    info->shadowsocks_opts[1] = NULL;
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

void args_decode(int argc, char **argv, bootstrap_info *info) { // decode the input parameters

    args_init(info);

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--debug")) { // --debug ==> debug mode
            info->is_debug = 1;
        } else if (!strcmp(argv[i], "--no-udp")) { // --no-udp ==> without udp proxy
            info->is_udp_proxy = 0;
        } else if (!strcmp(argv[i], "-c")) { // -c ==> CONFIG_JSON
            if (++i == argc) {
                log_fatal("`-c` require a parameter");
            }
            char *json_content = read_file(argv[i]);
            json_decode(json_content, info); // decode json content
            free(json_content);
        } else if (!strcmp(argv[i], "-s")) { // -s ==> server_addr
            if (++i == argc) {
                log_fatal("`-s` require a parameter");
            }
            if (info->server_addr != NULL) {
                free(info->server_addr); // override server address
            }
            info->server_addr = new_string(argv[i]);
        } else if (!strcmp(argv[i], "-p")) { // -p ==> server_port
            if (++i == argc) {
                log_fatal("`-p` require a parameter");
            }
            if (info->server_port != NULL) {
                free(info->server_port); // override server port
            }
            info->server_port = new_string(argv[i]);
        } else if (!strcmp(argv[i], "-b")) { // -b ==> client_addr
            if (++i == argc) {
                log_fatal("`-b` require a parameter");
            }
            if (info->client_addr != NULL) {
                free(info->client_addr); // override client address
            }
            info->client_addr = new_string(argv[i]);
        } else if (!strcmp(argv[i], "-l")) { // -l ==> client_port
            if (++i == argc) {
                log_fatal("`-l` require a parameter");
            }
            if (info->client_port != NULL) {
                free(info->client_port); // override client port
            }
            info->client_port = new_string(argv[i]);
        } else if (!strcmp(argv[i], "-k")) { // -k ==> password
            if (++i == argc) {
                log_fatal("`-k` require a parameter");
            }
            if (info->password != NULL) {
                free(info->password); // override password
            }
            info->password = new_string(argv[i]);
        } else if (!strcmp(argv[i], "-m")) { // -m ==> method
            if (++i == argc) {
                log_fatal("`-m` require a parameter");
            }
            if (info->method != NULL) {
                free(info->method); // override method
            }
            info->method = new_string(argv[i]);
        } else if (!strcmp(argv[i], "-t")) { // -t ==> timeout
            if (++i == argc) {
                log_fatal("`-t` require a parameter");
            }
            if (info->timeout != NULL) {
                free(info->timeout); // override timeout
            }
            info->timeout = new_string(argv[i]);
        } else if (!strcmp(argv[i], "--fast-open")) { // --fast-open
            info->fastopen = 1;
        } else if (!strcmp(argv[i], "--plugin")) { // --plugin ==> plugin
            if (++i == argc) {
                log_fatal("`--plugin` require a parameter");
            }
            if (info->plugin != NULL) {
                free(info->plugin); // override plugin
            }
            info->plugin = new_string(argv[i]);
        } else if (!strcmp(argv[i], "--plugin-opts")) { // --plugin-opts ==> plugin_opts
            if (++i == argc) {
                log_fatal("`--plugin-opts` require a parameter");
            }
            if (info->plugin_opts != NULL) {
                free(info->plugin_opts);
            }
            info->plugin_opts = new_string(argv[i]);
        } else if (!strcmp(argv[i], "--shadowsocks")) { // --shadowsocks ==> shadowsocks
            if (++i == argc) {
                log_fatal("`--shadowsocks` require a parameter");
            }
            if (info->shadowsocks != NULL) {
                free(info->shadowsocks);
            }
            info->shadowsocks = new_string(argv[i]);
        } else { // unknown option ==> archive
            add_shadowsocks_option(argv[i], info->shadowsocks_opts); // archive unknown options
        }
    }
    if (info->server_addr == NULL) { // default server address (bind address in server mode)
        info->server_addr = "127.0.0.1";
    }
    args_debug(info);
}

//void args_dump() { // show parameter's content
//
//}
