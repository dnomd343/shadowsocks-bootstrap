#include "load.h"
#include "logger.h"
#include "common.h"
#include "process.h"

char *help_msg = "\n\
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

int main(int argc, char *argv[]) {
    init(argc, argv, help_msg);
    log_info("Shadowsocks bootstrap server (%s)", VERSION);
    boot_info *info = load_info(argc, argv);

//    params_load("ssserver"); // default file name
//    start_bootstrap("ssserver", is_udp_proxy); // local or server mode
    return 0;
}
