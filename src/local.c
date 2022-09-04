#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "process.h"
#include "help.h"
#include "logger.h"
#include "load.h"

int main(int argc, char *argv[]) {
    is_show_help(argc, argv, local_help_msg);
    log_info("Shadowsocks bootstrap local (%s)", VERSION);

    // set log level
//    bootstrap_info info;
    load_info(argc, argv);
//    params_load("sslocal"); // default file name
//    start_bootstrap("sslocal", is_udp_proxy); // local or server mode
    return 0;
}
