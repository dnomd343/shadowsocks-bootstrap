#ifndef LOAD_H_
#define LOAD_H_

typedef struct {
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
} bootstrap;

bootstrap* load_info(int argc, char **argv);

#endif
