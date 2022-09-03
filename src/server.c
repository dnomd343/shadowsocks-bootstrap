#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "process.h"
#include "help.h"
#include "log.h"

int main(int argc, char *argv[]) {
    is_show_help(argc, argv, server_help_msg);
    log_info("Shadowsocks bootstrap server (%s)", VERSION);

    args_decode(argc, argv);
    params_load("ssserver"); // default file name
    start_bootstrap("ssserver", is_udp_proxy); // local or server mode
    return 0;
}
