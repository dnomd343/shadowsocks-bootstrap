#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "help.h"

char *server_help_msg = "\n\
ss-bootstrap-server\n\
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
    --shadowsocks <ssservre>   Set shadowsocks server program.\n\
    --no-udp                   Do not use UDP proxy.\n\
    -h, --help                 Print this message.\n\
\n\
";

char *local_help_msg = "\n\
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
    --no-udp                   Do not use UDP proxy.\n\
    -h, --help                 Print this message.\n\
\n\
";

void show_help(char *help_msg) {
    printf("%s", help_msg);
    exit(0); // exit normally
}

void is_show_help(int argc, char **argv, char *help_msg) { // output help when necessary
    if (argc <= 1) { // with only one argument
        show_help(help_msg);
    }
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { // include `-h` or `--help`
            show_help(help_msg);
        }
    }
}
