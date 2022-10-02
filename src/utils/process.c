#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include "network.h"
#include "process.h"
#include "logger.h"
#include "dns.h"

typedef struct {
    int pid;
    int exit_code;
    int exit_signal;
} exit_info;

char *plugin;
pid_t ss_pid = 0, plugin_pid = 0;
int exiting = 0; // sub process exiting
int exited = 0; // all sub process exited

void error_exit();
void normal_exit();
void get_sub_exit();
void kill_sub_process();
void show_exit_info(exit_info info, char *prefix);
exit_info get_exit_info(int status, pid_t pid);
char** load_plugin_env(sip003 *service);
void process_exec(sip003 *service);

void normal_exit() { // exit normally
    kill_sub_process();
    log_info("Exit complete");
    exit(0);
}

void error_exit() { // exit with error
    kill_sub_process();
    log_info("Exit with error");
    exit(1);
}

void show_exit_info(exit_info info, char *prefix) { // show info of child process death
    if (info.exit_code != -1) { // exit normally
        log_warn("%s (PID = %d) -> exit code %d", prefix, info.pid, info.exit_code);
    } else if (info.exit_signal != -1) { // abnormal exit
        log_warn("%s (PID = %d) -> killed by signal %d", prefix, info.pid, info.exit_signal);
    } else {
        log_warn("%s (PID = %d) -> unknown reason", prefix, info.pid);
    }
}

exit_info get_exit_info(int status, pid_t pid) { // get why the child process death
    exit_info temp;
    temp.pid = pid;
    temp.exit_code = temp.exit_signal = -1;
    if (WIFEXITED(status)) { // exit normally (with an exit-code)
        temp.exit_code = WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) { // abnormal exit (with a signal)
        temp.exit_signal = WTERMSIG(status);
    }
    return temp;
}

void kill_sub_process() { // kill child process
    while (exiting) {
        sleep(1); // exit process already working -> block
    }
    exiting = 1; // exit process flag
    PROXY_EXIT = 1; // udp proxy cancel
    if (ss_pid != 0) {
        kill(ss_pid, SIGKILL);
        log_info("Kill shadowsocks process");
    }
    if (plugin != NULL && plugin_pid != 0) {
        kill(plugin_pid, SIGKILL);
        log_info("Kill plugin process");
    }
    int status;
    log_info("Wait child process exit");
    waitpid(0, &status, 0); // block wait
}

void get_sub_exit() { // catch child process die
    exit_info sub_exit_info;
    int ss_ret, plugin_ret, ss_status, plugin_status;
    if (exiting) {
        int status;
        waitpid(0, &status, 0);
        return;
    }
    if (ss_pid != 0) {
        ss_ret = waitpid(ss_pid, &ss_status, WNOHANG); // non-blocking
        if (ss_ret == -1) {
            log_perror("Shadowsocks waitpid error -> ");
            error_exit();
        } else if (ss_ret) { // ss exit
            sub_exit_info = get_exit_info(ss_status, ss_pid);
            show_exit_info(sub_exit_info, "Shadowsocks exit");
            error_exit();
        }
    }
    if (plugin != NULL && plugin_pid != 0) { // with plugin
        plugin_ret = waitpid(plugin_pid, &plugin_status, WNOHANG); // non-blocking
        if (plugin_ret == -1) {
            log_perror("Plugin waitpid error -> ");
            error_exit();
        } else if (plugin_ret) { // plugin exit
            sub_exit_info = get_exit_info(plugin_status, plugin_pid);
            show_exit_info(sub_exit_info, "Plugin exit");
            error_exit();
        }
    }
    exited = 1;
}

char** load_plugin_env(sip003 *service) { // load plugin's environment variable
    char *remote_host = "SS_REMOTE_HOST=";
    char *remote_port = "SS_REMOTE_PORT=";
    char *local_host = "SS_LOCAL_HOST=";
    char *local_port = "SS_LOCAL_PORT=";
    char *plugin_options = "SS_PLUGIN_OPTIONS=";
    char **plugin_env = (char**)malloc(sizeof(char*) * 6);
    plugin_env[0] = (char*)malloc(strlen(remote_host) + strlen(service->SS_REMOTE_HOST) + 1);
    plugin_env[1] = (char*)malloc(strlen(remote_port) + strlen(service->SS_REMOTE_PORT) + 1);
    plugin_env[2] = (char*)malloc(strlen(local_host) + strlen(service->SS_LOCAL_HOST) + 1);
    plugin_env[3] = (char*)malloc(strlen(local_port) + strlen(service->SS_LOCAL_PORT) + 1);
    strcat(strcpy(plugin_env[0], remote_host), service->SS_REMOTE_HOST);
    strcat(strcpy(plugin_env[1], remote_port), service->SS_REMOTE_PORT);
    strcat(strcpy(plugin_env[2], local_host), service->SS_LOCAL_HOST);
    strcat(strcpy(plugin_env[3], local_port), service->SS_LOCAL_PORT);
    if (service->SS_PLUGIN_OPTIONS == NULL) {
        plugin_env[4] = NULL;
    } else {
        plugin_env[4] = (char*)malloc(strlen(plugin_options) + strlen(service->SS_PLUGIN_OPTIONS) + 1);
        strcat(strcpy(plugin_env[4], plugin_options), service->SS_PLUGIN_OPTIONS);
    }
    plugin_env[5] = NULL;
    return plugin_env;
}

void process_exec(sip003 *service) { // run shadowsocks main process and plugin
    if ((ss_pid = fork()) < 0) {
        log_perror("Shadowsocks fork error -> ");
        error_exit();
    } else if (ss_pid == 0) { // child process
        prctl(PR_SET_PDEATHSIG, SIGKILL); // child die with his father
        if (execvp(service->shadowsocks_cmd[0], service->shadowsocks_cmd) < 0) {
            log_perror("Shadowsocks exec error -> ");
            exit(2);
        }
    }
    plugin = service->plugin_file;
    if (plugin == NULL) { // plugin no need
        return;
    }
    usleep(100 * 1000); // sleep 100ms (python always a little slower)
    if (exiting) { // cancel plugin exec
        return;
    }
    if ((plugin_pid = fork()) < 0) {
        log_perror("Plugin fork error -> ");
        error_exit();
    } else if (plugin_pid == 0) { // child process
        prctl(PR_SET_PDEATHSIG, SIGKILL); // child die with his father
        char **plugin_env = load_plugin_env(service);
        char *plugin_arg[] = { plugin, NULL };
        if (execvpe(plugin, plugin_arg, plugin_env) < 0) {
            log_perror("Plugin exec error -> ");
            exit(2);
        }
    }
}

void start_bootstrap(int local_mode, sip003 *service) { // start shadowsocks (and plugin)
    log_info("Start shadowsocks bootstrap");
    signal(SIGINT, normal_exit); // catch Ctrl + C (2)
    signal(SIGTERM, normal_exit); // catch exit signal (15)
    signal(SIGCHLD, get_sub_exit); // callback when child process die
    process_exec(service); // exec child process

    if (service->plugin_file == NULL || !service->is_udp_proxy) { // start udp proxy when using plugin
        log_info("UDP Proxy no need");
    } else {
        char *remote_ip;
        if (is_ip_addr(service->SS_REMOTE_HOST)) { // remote_host -> ip address
            remote_ip = service->SS_REMOTE_HOST;
        } else { // remote_host -> domain
            log_info("DNS Resolve: %s", service->SS_REMOTE_HOST);
            remote_ip = dns_resolve(service->SS_REMOTE_HOST); // dns resolve
            if (remote_ip == NULL) { // no result
                log_warn("DNS record not found");
            } else { // dns resolve success
                log_info("%s => %s", service->SS_REMOTE_HOST, remote_ip);
            }
        }
        if (remote_ip == NULL) { // resolve error
            log_warn("Skip UDP Proxy");
        } else { // udp proxy
            if (local_mode) { // local mode
                proxy(remote_ip, atoi(service->SS_REMOTE_PORT), service->SS_LOCAL_HOST, atoi(service->SS_LOCAL_PORT)); // NOLINT
            } else { // server mode
                proxy(service->SS_LOCAL_HOST, atoi(service->SS_LOCAL_PORT), remote_ip, atoi(service->SS_REMOTE_PORT)); // NOLINT
            }
        }
    }
    while (!exited) {
        pause();
    }
    normal_exit();
}
