#include <stdlib.h>
#include "sip003.h"
#include "logger.h"
#include "common.h"
#include "network.h"

void dump_sip003(sip003 *service);
void add_sip003_arg(bootstrap *info, char *key, char *value);

void dump_sip003(sip003 *service) { // show shadowsocks and plugin params
    char *cmd_str = string_list_join(service->shadowsocks_cmd);
    log_info("Shadowsocks -> %s", cmd_str);
    free(cmd_str);
    if (service->plugin_file == NULL) {
        log_info("Plugin no need");
    } else {
        log_info("Plugin -> `%s`", service->plugin_file);
        log_info("SS_REMOTE_HOST -> `%s`", service->SS_REMOTE_HOST);
        log_info("SS_REMOTE_PORT -> `%s`", service->SS_REMOTE_PORT);
        log_info("SS_LOCAL_HOST -> `%s`", service->SS_LOCAL_HOST);
        log_info("SS_LOCAL_PORT -> `%s`", service->SS_LOCAL_PORT);
        log_info("SS_PLUGIN_OPTIONS -> `%s`", service->SS_PLUGIN_OPTIONS);
    }
}

void add_sip003_arg(bootstrap *info, char *key, char *value) {
    if (value != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, key);
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, value);
    }
}

sip003* load_sip003(char *ss_default, bootstrap *info) { // load shadowsocks and plugin params
    sip003 *service = (sip003*)malloc(sizeof(sip003));
    if (info->shadowsocks == NULL) {
        info->shadowsocks = ss_default; // load default shadowsocks name
    }
    info->shadowsocks_opts[0] = info->shadowsocks; // fill with file name

    service->plugin_file = NULL;
    if (info->plugin != NULL) { // with sip003 plugin
        char *rand_port = int_to_string(get_available_port(RANDOM_PORT_START, RANDOM_PORT_END));
        service->SS_REMOTE_HOST = info->server_addr;
        service->SS_REMOTE_PORT = info->server_port;
        service->SS_LOCAL_HOST = "127.0.0.1";
        service->SS_LOCAL_PORT = rand_port;
        service->SS_PLUGIN_OPTIONS = info->plugin_opts;
        info->server_addr = service->SS_LOCAL_HOST;
        info->server_port = service->SS_LOCAL_PORT;
        service->plugin_file = info->plugin;
    }

    add_sip003_arg(info, "-s", info->server_addr);
    add_sip003_arg(info, "-b", info->client_addr);
    add_sip003_arg(info, "-p", info->server_port);
    add_sip003_arg(info, "-l", info->client_port);
    add_sip003_arg(info, "-k", info->password);
    add_sip003_arg(info, "-m", info->method);
    add_sip003_arg(info, "-t", info->timeout);
    if (info->fastopen) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "--fast-open");
    }
    service->shadowsocks_cmd = info->shadowsocks_opts;
    dump_sip003(service);
    return service;
}
