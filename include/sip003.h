#ifndef SIP003_H_
#define SIP003_H_

#include "load.h"

typedef struct {
    char *SS_REMOTE_HOST;
    char *SS_REMOTE_PORT;
    char *SS_LOCAL_HOST;
    char *SS_LOCAL_PORT;
    char *SS_PLUGIN_OPTIONS;
    char *plugin_file;
    char **shadowsocks_cmd;
    int is_udp_proxy;
} sip003;

sip003* load_sip003(char *ss_default, bootstrap *info);

#endif
