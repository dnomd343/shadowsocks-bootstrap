#ifndef _COMMON_H_
#define _COMMON_H_

#define RANDOM_PORT_START 41952
#define RANDOM_PORT_END   65535

extern char *server_addr, *client_addr;
extern char *server_port, *client_port;
extern char *password;
extern char *method;
extern char *timeout;
extern int fastopen;
extern char *plugin;
extern char *plugin_opts;
extern char *shadowsocks;
extern char **shadowsocks_opts;

void params_load(char *ss_default);
void args_decode(int argc, char **argv);

#endif
