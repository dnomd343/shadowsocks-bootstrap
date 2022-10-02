#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "logger.h"
#include "cJSON.h"
#include "load.h"

void init_info(bootstrap *info);
void dump_info(bootstrap *info);
void json_decode(char *json_content, bootstrap *info);
char** add_extra_opts(char **opts, char *extra_opts_str);
int add_field(char *field, char **target, char ***arg, char ***arg_limit);

void init_info(bootstrap *info) {
    info->is_udp_proxy = TRUE; // enabled udp proxy
    info->server_addr = info->client_addr = NULL;
    info->server_port = info->client_port = NULL;
    info->password = NULL;
    info->method = NULL;
    info->timeout = NULL;
    info->fastopen = FALSE;
    info->plugin = NULL;
    info->plugin_opts = NULL;
    info->shadowsocks = NULL;
    info->shadowsocks_opts = (char**)malloc(sizeof(char*) * 2); // two arguments
    info->shadowsocks_opts[0] = ""; // reserved for program name
    info->shadowsocks_opts[1] = NULL;
}

void dump_info(bootstrap *info) {
    if (info->is_udp_proxy) {
        log_debug("is_udp_proxy = true");
    } else {
        log_debug("is_udp_proxy = false");
    }
    log_debug("server_addr = %s", info->server_addr);
    log_debug("client_addr = %s", info->client_addr);
    log_debug("server_port = %s", info->server_port);
    log_debug("client_port = %s", info->client_port);
    log_debug("password = %s", info->password);
    log_debug("method = %s", info->method);
    log_debug("timeout = %s", info->timeout);
    if (info->fastopen) {
        log_debug("fastopen = true");
    } else {
        log_debug("fastopen = false");
    }
    log_debug("plugin = %s", info->plugin);
    log_debug("plugin_opts = %s", info->plugin_opts);
    log_debug("shadowsocks = %s", info->shadowsocks);

    char *opts_str = string_list_join(info->shadowsocks_opts + 1);
    log_debug("shadowsocks_opts: %s", opts_str); // combine output
    free(opts_str);
}

char** add_extra_opts(char **opts, char *extra_opts_str) { // split shadowsocks extra options
    log_debug("Split extra options -> `%s`", extra_opts_str);
    char *tmp = (char*)calloc(strlen(extra_opts_str) + 1, 1); // new memory and set as 0x00
    for (int i = 0, num = 0;; ++num) {
        if (extra_opts_str[num] == '\0' || extra_opts_str[num] == ' ') { // string end or find a space
            tmp[i] = '\0';
            if (i) { // ignore empty string
                i = 0;
                opts = string_list_append(opts, tmp);
            }
            if (extra_opts_str[num] == '\0') { // string end
                break;
            }
            continue;
        }
        if (extra_opts_str[num] == '\\') { // skip '\' char
            ++num;
        }
        tmp[i++] = extra_opts_str[num]; // copy char
    }
    return opts;
}

void json_decode(char *json_content, bootstrap *info) { // decode JSON content
    cJSON* json = cJSON_Parse(json_content);
    if (json == NULL) {
        log_fatal("JSON format error");
    }
    json = json->child;
    while (json != NULL) {
        if (!strcmp(json->string, "no_udp")) { // no_udp => without udp proxy
            if (!cJSON_IsBool(json)) {
                log_fatal("`no_udp` must be bool");
            }
            if (json->valueint) { // is_udp_proxy = ~(json->valueint)
                info->is_udp_proxy = FALSE;
            } else {
                info->is_udp_proxy = TRUE;
            }
        } else if (!strcmp(json->string, "server")) { // server => server_addr
            if (!cJSON_IsString(json)) {
                log_fatal("`server` must be string");
            }
            if (info->server_addr != NULL) {
                free(info->server_addr);
            }
            info->server_addr = new_string(json->valuestring);
        } else if (!strcmp(json->string, "local_address")) { // local_address => client_addr
            if (!cJSON_IsString(json)) {
                log_fatal("`local_address` must be string");
            }
            if (info->client_addr != NULL) {
                free(info->client_addr);
            }
            info->client_addr = new_string(json->valuestring);
        } else if (!strcmp(json->string, "server_port")) { // server_port => server_port
            if (info->server_port != NULL) {
                free(info->server_port);
            }
            if (cJSON_IsNumber(json)) {
                info->server_port = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                info->server_port = new_string(json->valuestring);
            } else {
                log_fatal("`server_port` must be number or string");
            }
        } else if (!strcmp(json->string, "local_port")) { // local_port => client_port
            if (info->client_port != NULL) {
                free(info->client_port);
            }
            if (cJSON_IsNumber(json)) {
                info->client_port = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                info->client_port = new_string(json->valuestring);
            } else {
                log_fatal("`local_port` must be number or string");
            }
        } else if (!strcmp(json->string, "password")) { // password => password
            if (!cJSON_IsString(json)) {
                log_fatal("`password` must be string");
            }
            if (info->password != NULL) {
                free(info->password);
            }
            info->password = new_string(json->valuestring);
        } else if (!strcmp(json->string, "timeout")) { // timeout => timeout
            if (info->timeout != NULL) {
                free(info->timeout);
            }
            if (cJSON_IsNumber(json)) {
                info->timeout = int_to_string(json->valueint);
            } else if (cJSON_IsString(json)) {
                info->timeout = new_string(json->valuestring);
            } else {
                log_fatal("`timeout` must be number or string");
            }
        } else if (!strcmp(json->string, "method")) { // method => method
            if (!cJSON_IsString(json)) {
                log_fatal("`method` must be string");
            }
            if (info->method != NULL) {
                free(info->method);
            }
            info->method = new_string(json->valuestring);
        } else if (!strcmp(json->string, "fast_open")) { // fast_open => fastopen
            if (!cJSON_IsBool(json)) {
                log_fatal("`fast_open` must be bool");
            }
            info->fastopen = json->valueint;
        } else if (!strcmp(json->string, "plugin")) { // plugin => plugin
            if (!cJSON_IsString(json)) {
                log_fatal("`plugin` must be string");
            }
            if (info->plugin != NULL) {
                free(info->plugin);
            }
            info->plugin = new_string(json->valuestring);
        } else if (!strcmp(json->string, "plugin_opts")) { // plugin_opts => plugin_opts
            if (!cJSON_IsString(json)) {
                log_fatal("`plugin_opts` must be string");
            }
            if (info->plugin_opts != NULL) {
                free(info->plugin_opts);
            }
            info->plugin_opts = new_string(json->valuestring);
        } else if (!strcmp(json->string, "shadowsocks")) { // shadowsocks => shadowsocks
            if (!cJSON_IsString(json)) {
                log_fatal("`shadowsocks` must be string");
            }
            if (info->shadowsocks != NULL) {
                free(info->shadowsocks);
            }
            info->shadowsocks = new_string(json->valuestring);
        } else if (!strcmp(json->string, "extra_opts")) { // extra_opts => DECODE => shadowsocks_opts
            if (!cJSON_IsString(json)) {
                log_fatal("`extra_opts` must be string");
            }
            info->shadowsocks_opts = add_extra_opts(info->shadowsocks_opts, json->valuestring);
        } else { // unknown field => ERROR
            log_fatal("Unknown JSON field `%s`", json->string);
        }
        json = json->next; // next field
    }
    cJSON_free(json); // free JSON struct
}

int add_field(char *field, char **target, char ***arg, char ***arg_limit) {
    if (strcmp(**arg, field) != 0) { // field not match
        return FALSE;
    }
    if (++(*arg) == *arg_limit) { // without next argument
        log_fatal("`%s` require a parameter", field);
    }
    if (*target != NULL) {
        free(*target); // override target field
    }
    *target = new_string(**arg);
    return TRUE;
}

bootstrap* load_info(int argc, char **argv) { // load info from input parameters
    bootstrap *info = (bootstrap*)malloc(sizeof(bootstrap));
    char **arg_limit = argv + argc;

    log_debug("Start to load input arguments");
    char *arg_str = (char*)malloc(0);
    for (char **arg = argv + 1; arg < arg_limit; ++arg) {
        arg_str = (char*)realloc(arg_str, strlen(arg_str) + strlen(*arg) + 4);
        arg_str = strcat(strcat(strcat(arg_str, "`"), *arg), "` ");
    }
    log_debug("Input arguments -> %s", arg_str);

    init_info(info);
    for (char **arg = argv + 1; arg < arg_limit; ++arg) {
        if (!strcmp(*arg, "--debug")) { // skip debug flag
            continue;
        } else if (!strcmp(*arg, "--fast-open")) { // --fast-open => fastopen
            info->fastopen = TRUE;
        } else if (!strcmp(*arg, "--no-udp")) { // --no-udp => is_udp_proxy
            info->is_udp_proxy = FALSE;
        } else if (!strcmp(*arg, "-c")) { // -c => CONFIG_JSON
            if (++arg == arg_limit) {
                log_fatal("Miss json file after `-c` flag");
            }
            char *json_content = read_file(*arg);
            json_decode(json_content, info); // decode json content
            free(json_content);
        } else if (
            !add_field("-s", &info->server_addr, &arg, &arg_limit) &&
            !add_field("-p", &info->server_port, &arg, &arg_limit) &&
            !add_field("-b", &info->client_addr, &arg, &arg_limit) &&
            !add_field("-l", &info->client_port, &arg, &arg_limit) &&
            !add_field("-k", &info->password, &arg, &arg_limit) &&
            !add_field("-m", &info->method, &arg, &arg_limit) &&
            !add_field("-t", &info->timeout, &arg, &arg_limit) &&
            !add_field("--plugin", &info->plugin, &arg, &arg_limit) &&
            !add_field("--plugin-opts", &info->plugin_opts, &arg, &arg_limit) &&
            !add_field("--shadowsocks", &info->shadowsocks, &arg, &arg_limit)
        ) { // archive unknown options
            info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, *arg);
        }
    }
    if (info->server_addr == NULL) { // default server address (bind address in server mode)
        info->server_addr = "127.0.0.1";
    }
    dump_info(info);
    return info;
}
