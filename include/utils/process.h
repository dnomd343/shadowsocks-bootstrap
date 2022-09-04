#ifndef _PROCESS_H_
#define _PROCESS_H_

extern char **shadowsocks_args;
extern char *plugin_file;
extern char *SS_REMOTE_HOST;
extern char *SS_REMOTE_PORT;
extern char *SS_LOCAL_HOST;
extern char *SS_LOCAL_PORT;
extern char *SS_PLUGIN_OPTIONS;

void start_bootstrap(char *ss_type, int is_udp_proxy);

#endif
