#include "load.h"
#include "logger.h"
#include "common.h"
#include "process.h"

#define HELP_MSG "\n\
ss-bootstrap-local (%s)\n\
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
    --debug                    Enable debug mode.\n\
    -h, --help                 Print this message.\n\
\n"

int main(int argc, char *argv[]) {
    init(argc, argv, HELP_MSG);
    log_info("Shadowsocks bootstrap local (%s)", VERSION);
    bootstrap *info = load_info(argc, argv);
    start_bootstrap(TRUE, load_sip003("sslocal", info));
    return 0;
}
