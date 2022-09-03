#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "process.h"
#include "help.h"
#include "log.h"

int main(int argc, char *argv[]) {
    is_show_help(argc, argv, local_help_msg);
    log_info("Shadowsocks bootstrap local (%s)", VERSION);

    bootstrap_info info;
    args_decode(argc, argv, &info);
//    params_load("sslocal"); // default file name
//    start_bootstrap("sslocal", is_udp_proxy); // local or server mode
    return 0;
}
