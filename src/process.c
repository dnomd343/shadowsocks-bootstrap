#define _GNU_SOURCE
#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include "network.h"
#include "process.h"
#include "dns.h"

char **shadowsocks_args;
char *plugin_file;
char *SS_REMOTE_HOST;
char *SS_REMOTE_PORT;
char *SS_LOCAL_HOST;
char *SS_LOCAL_PORT;
char *SS_PLUGIN_OPTIONS;

char **plugin_env;
char *plugin_arg[] = { NULL, NULL };

int exiting = 0;

typedef struct exit_info {
    int pid;
    int exit_code;
    int exit_signal;
} exit_info;

GMainLoop* main_loop;
pid_t ss_pid = 0, plugin_pid = 0;

void show_params();
void process_exec();
void get_sub_exit();
void exit_with_child();
void plugin_env_load();
void show_exit_info(exit_info info);
exit_info get_exit_info(int status, pid_t pid);

void show_exit_info(exit_info info) { // show info of child process death
    printf("(PID = %d) -> ", info.pid);
    if (info.exit_code != -1) { // exit normally
        printf("exit code %d.\n", info.exit_code);
    } else if (info.exit_signal != -1) { // abnormal exit
        printf("killed by signal %d.\n", info.exit_signal);
    } else {
        printf("unknown reason.\n");
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
            perror("[Shadowsocks Bootstrap] shadowsocks waitpid error");
            exit_with_child();
        } else if (ss_ret) { // ss exit
            sub_exit_info = get_exit_info(ss_status, ss_pid);
            printf("[Shadowsocks Bootstrap] shadowsocks exit ");
            show_exit_info(sub_exit_info);
            exit_with_child();
        }
    }
    if (plugin_file != NULL && plugin_pid != 0) { // with plugin
        plugin_ret = waitpid(plugin_pid, &plugin_status, WNOHANG); // non-blocking
        if (plugin_ret == -1) {
            perror("[Shadowsocks Bootstrap] plugin waitpid error");
            exit_with_child();
        } else if (plugin_ret) { // plugin exit
            sub_exit_info = get_exit_info(plugin_status, plugin_pid);
            printf("[Shadowsocks Bootstrap] plugin exit ");
            show_exit_info(sub_exit_info);
            exit_with_child();
        }
    }
    g_main_loop_quit(main_loop); // exit main loop
}

void plugin_env_load() { // load plugin's environment variable
    char *remote_host = "SS_REMOTE_HOST=";
    char *remote_port = "SS_REMOTE_PORT=";
    char *local_host = "SS_LOCAL_HOST=";
    char *local_port = "SS_LOCAL_PORT=";
    char *plugin_options = "SS_PLUGIN_OPTIONS=";
    plugin_env = (char**)malloc(sizeof(char*) * 6);
    plugin_env[0] = (char*)malloc(strlen(remote_host) + strlen(SS_REMOTE_HOST) + 1);
    plugin_env[1] = (char*)malloc(strlen(remote_port) + strlen(SS_REMOTE_PORT) + 1);
    plugin_env[2] = (char*)malloc(strlen(local_host) + strlen(SS_LOCAL_HOST) + 1);
    plugin_env[3] = (char*)malloc(strlen(local_port) + strlen(SS_LOCAL_PORT) + 1);
    strcat(strcpy(plugin_env[0], remote_host), SS_REMOTE_HOST);
    strcat(strcpy(plugin_env[1], remote_port), SS_REMOTE_PORT);
    strcat(strcpy(plugin_env[2], local_host), SS_LOCAL_HOST);
    strcat(strcpy(plugin_env[3], local_port), SS_LOCAL_PORT);
    if (SS_PLUGIN_OPTIONS == NULL) {
        plugin_env[4] = NULL;
    } else {
        plugin_env[4] = (char*)malloc(strlen(plugin_options) + strlen(SS_PLUGIN_OPTIONS) + 1);
        strcat(strcpy(plugin_env[4], plugin_options), SS_PLUGIN_OPTIONS);
    }
    plugin_env[5] = NULL;
    plugin_arg[0] = plugin_file;
}

void process_exec() { // run shadowsocks main process and plugin (as child process)
    if ((ss_pid = fork()) < 0) {
        perror("[Shadowsocks Bootstrap] fork error");
        exit_with_child();
    } else if (ss_pid == 0) { // child process
        prctl(PR_SET_PDEATHSIG, SIGKILL); // child die with his father
        if (execvp(shadowsocks_args[0], shadowsocks_args) < 0) {
            perror("[Shadowsocks Bootstrap] shadowsocks exec error");
            exit(2);
        }
    }
    if (plugin_file == NULL) { // plugin no need
        return;
    }
    usleep(100 * 1000); // sleep 100ms (python always a little slower)
    if (exiting) { // cancel plugin exec
        return;
    }
    if ((plugin_pid = fork()) < 0) {
        perror("[Shadowsocks Bootstrap] fork error");
        exit_with_child();
    } else if (plugin_pid == 0) { // child process
        prctl(PR_SET_PDEATHSIG, SIGKILL); // child die with his father
        plugin_env_load();
        if (execvpe(plugin_file, plugin_arg, plugin_env) < 0) {
            perror("[Shadowsocks Bootstrap] plugin exec error");
            exit(2);
        }
    }
}

void exit_with_child() { // exit and kill his child process
    while (exiting) {
        sleep(1); // block
    }
    exiting = 1;
    proxy_exit = 1;
    if (ss_pid != 0) {
        kill(ss_pid, SIGKILL);
        printf("[Shadowsocks Bootstrap] kill shadowsocks process.\n");
    }
    if (plugin_file != NULL && plugin_pid != 0) {
        kill(plugin_pid, SIGKILL);
        printf("[Shadowsocks Bootstrap] kill plugin process.\n");
    }
    int status;
    printf("[Shadowsocks Bootstrap] wait for child process.\n");
    waitpid(0, &status, 0); // block
    printf("[Shadowsocks Bootstrap] exit.\n");
    exit(1);
}

void show_params() { // show shadowsocks and plugin params
    int num = 0;
    printf("[Shadowsocks Bootstrap]");
    while(shadowsocks_args[num] != NULL) {
        printf(" %s", shadowsocks_args[num++]);
    }
    printf("\n");
    if (plugin_file == NULL) {
        printf("[Shadowsocks Bootstrap] Plugin no need.\n");
        return;
    }
    printf("[Shadowsocks Bootstrap] Plugin -> %s\n", plugin_file);
    printf("[Shadowsocks Bootstrap]   SS_REMOTE_HOST -> %s\n", SS_REMOTE_HOST);
    printf("[Shadowsocks Bootstrap]   SS_REMOTE_PORT -> %s\n", SS_REMOTE_PORT);
    printf("[Shadowsocks Bootstrap]   SS_LOCAL_HOST -> %s\n", SS_LOCAL_HOST);
    printf("[Shadowsocks Bootstrap]   SS_LOCAL_PORT -> %s\n", SS_LOCAL_PORT);
    printf("[Shadowsocks Bootstrap]   SS_PLUGIN_OPTIONS -> %s\n", SS_PLUGIN_OPTIONS);
}

void start_bootstrap(char *ss_type, int is_udp_proxy) { // start shadowsocks and plugin (optional)
    show_params();
    main_loop = g_main_loop_new(NULL, FALSE);
    signal(SIGINT, exit_with_child); // catch Ctrl + C (2)
    signal(SIGTERM, exit_with_child); // catch exit signal (15)
    signal(SIGCHLD, get_sub_exit); // callback when child process die
    process_exec(); // exec child process
    if (plugin_file != NULL && is_udp_proxy) { // start udp proxy when using plugin
        char *remote_ip;
        if (is_ip_addr(SS_REMOTE_HOST)) { // remote_host -> ip address
            remote_ip = SS_REMOTE_HOST;
        } else { // remote_host -> domain
            printf("[Shadowsocks Bootstrap] DNS Resolve: %s\n", SS_REMOTE_HOST);
            remote_ip = dns_resolve(SS_REMOTE_HOST); // dns resolve
            if (remote_ip == NULL) { // no result
                printf("[Shadowsocks Bootstrap] DNS record not found.\n");
            } else { // dns resolve success
                printf("[Shadowsocks Bootstrap] %s => %s\n", SS_REMOTE_HOST, remote_ip);
            }
        }
        if (remote_ip == NULL) { // resolve error
            printf("[Shadowsocks Bootstrap] Skip UDP Proxy.\n");
        } else { // udp proxy
            if (!strcmp(ss_type, "sslocal")) { // local mode
                proxy(remote_ip, atoi(SS_REMOTE_PORT), SS_LOCAL_HOST, atoi(SS_LOCAL_PORT));
            } else { // server mode
                proxy(SS_LOCAL_HOST, atoi(SS_LOCAL_PORT), remote_ip, atoi(SS_REMOTE_PORT));
            }
        }
    } else {
        printf("[Shadowsocks Bootstrap] UDP Proxy no need.\n");
    }
    g_main_loop_run(main_loop); // into main loop for wait
    exit_with_child();
}
