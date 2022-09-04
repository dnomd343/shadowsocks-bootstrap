#ifndef _COMMON_H_
#define _COMMON_H_

#define VERSION "0.9.1"

#define RANDOM_PORT_START 41952
#define RANDOM_PORT_END   65535

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
} bootstrap_info;

char* int_to_string(int num);
char* new_string(char *str);
char* read_file(char *file_name);
void params_load(char *ss_default, bootstrap_info *info);
void args_decode(int argc, char **argv, bootstrap_info *info);

#endif
