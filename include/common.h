#ifndef _COMMON_H_
#define _COMMON_H_

#define RANDOM_PORT_START 41952
#define RANDOM_PORT_END   65535

//extern int is_udp_proxy;
//extern char *server_addr, *client_addr;
//extern char *server_port, *client_port;
//extern char *password;
//extern char *method;
//extern char *timeout;
//extern int fastopen;
//extern char *plugin;
//extern char *plugin_opts;
//extern char *shadowsocks;
//extern char **shadowsocks_opts;

typedef struct {
    int is_debug, is_udp_proxy;
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
} bootstrap_info;

void params_load(char *ss_default, bootstrap_info *info);
void args_decode(int argc, char **argv, bootstrap_info *info);

#endif
