#include "sip003.h"
#include "load.h"

void params_load(char *ss_default, boot_info *info) { // load shadowsocks and plugin params
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

void pack_shadowsocks_params(boot_info *info) { // packaging shadowsocks parameters
    if (info->server_addr != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-s");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->server_addr);
    }
    if (info->client_addr != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-b");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->client_addr);
    }
    if (info->server_port != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-p");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->server_port);
    }
    if (info->client_port != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-l");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->client_port);
    }
    if (info->password != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-k");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->password);
    }
    if (info->method != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-m");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->method);
    }
    if (info->timeout != NULL) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "-t");
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, info->timeout);
    }
    if (info->fastopen) {
        info->shadowsocks_opts = string_list_append(info->shadowsocks_opts, "--fast-open");
    }
}