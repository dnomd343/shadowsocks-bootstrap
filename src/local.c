#include <stdio.h>
#include <string.h>
#include "common.h"
#include "process.h"

#define SHADOWSOCKS_DEFAULT "sslocal"

char *help_msg =
"\
\n\
ss-bootstrap-local\n\
\n\
    A simple program to make the original shadowsocks support SIP003 plugins.\n\
\n\
    -s <server_host>           Host name or IP address of your remote server.\n\
    -p <server_port>           Port number of your remote server.\n\
    -b <local_address>         Local address to bind.\n\
    -l <local_port>            Port number of your local server.\n\
\n\
    -c <config_file>           Path to JSON config file.\n\
    -k <password>              Password of your remote server.\n\
    -m <method>                Encrypt method.\n\
    -t <timeout>               Socket timeout in seconds.\n\
    --fast-open                Enable TCP fast open (with Linux kernel 3.7+).\n\
    --plugin <name>            Enable SIP003 plugin.\n\
    --plugin-opts <options>    Set SIP003 plugin options.\n\
    --shadowsocks <sslocal>    Set shadowsocks local program.\n\
    -h, --help                 Print this message.\n\
\n\
";

int main(int argc, char *argv[]) {
    int i = 0;
    for (i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("%s", help_msg); // show help message
            return 0;
        }
    }
    args_decode(argc, argv);
    params_load(SHADOWSOCKS_DEFAULT); // default file name
    start_bootstrap();
    return 0;
}
