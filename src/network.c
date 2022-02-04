#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "network.h"

#define TIMEOUT 15
#define BUFFER_SIZE 4096

int proxy_exit;

int get_random_num(int range_start, int range_end);
int check_port_available(unsigned int port, int is_udp, int is_ipv6);
int is_ipv4_addr(char *address);
int is_ipv6_addr(char *address);
int create_ipv4_udp_sock(char *address, int port);
int create_ipv6_udp_sock(char *address, int port);
long ipv4_receive(int fd, char *buffer, int buffer_size, int timeout, struct sockaddr_in sa);
long ipv6_receive(int fd, char *buffer, int buffer_size, int timeout, struct sockaddr_in6 sa);
long ipv4_send_and_receive(char *ipv4_server_ip, int ipv4_server_port, char *send_buffer, long send_len, char *recv_buffer);
long ipv6_send_and_receive(char *ipv6_server_ip, int ipv6_server_port, char *send_buffer, long send_len, char *recv_buffer);
long send_and_receive(char *server_ip, int server_port, char *send_buffer, long send_len, char *recv_buffer);
void ipv4_proxy(void *ipv4_info);
void ipv6_proxy(void *ipv6_info);

typedef struct ipv4_proxy_info {
    char *server_ip;
    int server_port;
    struct sockaddr_in ipv4_client_addr;
    int ipv4_client_fd;
    char *buffer;
    long len;
} ipv4_proxy_info;

typedef struct ipv6_proxy_info {
    char *server_ip;
    int server_port;
    struct sockaddr_in6 ipv6_client_addr;
    int ipv6_client_fd;
    char *buffer;
    long len;
} ipv6_proxy_info;

int get_random_num(int range_start, int range_end) { // create a random number in range
    struct timeval tp;
    gettimeofday(&tp, NULL);
    srand(tp.tv_usec);
    return range_start + (rand() % (range_end - range_start + 1));
}

int check_port_available(unsigned int port, int is_udp, int is_ipv6) { // test a port is available or not
    int ipv4_tcp_sock, ipv4_udp_sock;
    int ipv6_tcp_sock, ipv6_udp_sock;
    struct sockaddr_in ipv4_tcp_addr, ipv4_udp_addr;
    struct sockaddr_in6 ipv6_tcp_addr, ipv6_udp_addr;

    ipv4_tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&ipv4_tcp_addr, sizeof(ipv4_tcp_addr));
    ipv4_tcp_addr.sin_family = AF_INET;
    ipv4_tcp_addr.sin_port = htons(port);
    ipv4_tcp_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(ipv4_tcp_sock, (struct sockaddr*)&ipv4_tcp_addr, sizeof(ipv4_tcp_addr)) < 0) {
        return 0; // false
    }
    close(ipv4_tcp_sock);

    if (is_udp) { // udp check
        ipv4_udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
        bzero(&ipv4_udp_addr, sizeof(ipv4_udp_addr));
        ipv4_udp_addr.sin_family = AF_INET;
        ipv4_udp_addr.sin_port = htons(port);
        ipv4_udp_addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(ipv4_udp_sock, (struct sockaddr*)&ipv4_udp_addr, sizeof(ipv4_udp_addr)) < 0) {
            return 0; // false
        }
        close(ipv4_udp_sock);
    }

    if (!is_ipv6) { // ipv6 ignore
        return 1; // true
    }

    ipv6_tcp_sock = socket(AF_INET6, SOCK_STREAM, 0);
    bzero(&ipv6_tcp_addr, sizeof(ipv6_tcp_addr));
    ipv6_tcp_addr.sin6_family = AF_INET6;
    ipv6_tcp_addr.sin6_port = htons(port);
    ipv6_tcp_addr.sin6_addr = in6addr_any;
    if (bind(ipv6_tcp_sock, (struct sockaddr*)&ipv6_tcp_addr, sizeof(ipv6_tcp_addr)) < 0) {
        return 0; // false
    }
    close(ipv6_tcp_sock);

    if (is_udp) { // udp check
        ipv6_udp_sock = socket(AF_INET6, SOCK_DGRAM, 0);
        bzero(&ipv6_udp_addr, sizeof(ipv6_udp_addr));
        ipv6_udp_addr.sin6_family = AF_INET6;
        ipv6_udp_addr.sin6_port = htons(port);
        ipv6_udp_addr.sin6_addr = in6addr_any;
        if (bind(ipv6_udp_sock, (struct sockaddr*)&ipv6_udp_addr, sizeof(ipv6_udp_addr)) < 0) {
            return 0; // false
        }
        close(ipv6_udp_sock);
    }

    return 1; // true
}

int get_available_port(unsigned short range_start, unsigned short range_end) { // get a available port
    unsigned short port;
    for (;;) { // wait until a available port in range
        port = get_random_num(range_start, range_end); // get a random port in range
        if (check_port_available(port, 1, 1)) { // port available
            return (int)port;
        }
    }
}

int is_ipv4_addr(char *address) { // 判断是否为IPv4地址
    if (inet_addr(address) == -1) {
        return 0;
    }
    return 1;
}

int is_ipv6_addr(char *address) { // 判断是否为IPv6地址
    char buf[sizeof(struct in6_addr)];
    if (inet_pton(AF_INET6, address, buf) <= 0) {
        return 0;
    }
    return 1;
}

int is_ip_addr(char *address) { // 判断是否为IP地址
    if (is_ipv4_addr(address) || is_ipv6_addr(address)) {
        return 1;
    }
    return 0;
}

int create_ipv4_udp_sock(char *address, int port) { // 创建并绑定IPv4 UDP端口
    struct sockaddr_in ipv4_udp_addr;
    int ipv4_udp_sock = socket(AF_INET, SOCK_DGRAM, 0); // IPv4 UDP模式
    bzero(&ipv4_udp_addr, sizeof(ipv4_udp_addr)); // 清空为0x00
    ipv4_udp_addr.sin_family = AF_INET;
    ipv4_udp_addr.sin_port = htons(port); // 监听端口
    if (address == NULL) {
        ipv4_udp_addr.sin_addr.s_addr = INADDR_ANY; // 监听0.0.0.0
    } else {
        ipv4_udp_addr.sin_addr.s_addr = inet_addr(address); // 监听地址
    }
    if (bind(ipv4_udp_sock, (struct sockaddr*)&ipv4_udp_addr, sizeof(ipv4_udp_addr)) < 0) { // 绑定接口
        perror("[Shadowsocks Bootstrap] IPv4 UDP Sock bind error");
        return -1; // 端口被占用
    }
    return ipv4_udp_sock;
}

int create_ipv6_udp_sock(char *address, int port) { // 创建并绑定IPv6 UDP端口
    struct sockaddr_in6 ipv6_udp_addr;
    int ipv6_udp_sock = socket(AF_INET6, SOCK_DGRAM, 0); // IPv6 UDP模式
    bzero(&ipv6_udp_addr, sizeof(ipv6_udp_addr)); // 清空为0x00
    ipv6_udp_addr.sin6_family = AF_INET6;
    ipv6_udp_addr.sin6_port = htons(port); // 监听端口
    if (address == NULL) {
        ipv6_udp_addr.sin6_addr = in6addr_any; // 监听::
    } else {
        inet_pton(AF_INET6, address, &ipv6_udp_addr.sin6_addr); // 监听地址
    }
    if (bind(ipv6_udp_sock, (struct sockaddr*)&ipv6_udp_addr, sizeof(ipv6_udp_addr)) < 0) { // 绑定接口
        perror("[Shadowsocks Bootstrap] IPv6 UDP Sock bind error");
        return -1; // 端口被占用
    }
    return ipv6_udp_sock;
}

long ipv4_receive(int fd, char *buffer, int buffer_size, int timeout, struct sockaddr_in sa) { // IPv4接收 超时处理
    socklen_t sa_len = sizeof(sa);
    if (timeout == 0) { // 永久等待
        return recvfrom(fd, buffer, buffer_size, 0, (struct sockaddr*)&sa, &sa_len);
    }
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = timeout; // 超时时间 单位s
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    select(fd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
    if (FD_ISSET(fd, &rfds)) {
        return recvfrom(fd, buffer, buffer_size, 0, (struct sockaddr*)&sa, &sa_len);
    }
    return -1; // 接收超时
}

long ipv6_receive(int fd, char *buffer, int buffer_size, int timeout, struct sockaddr_in6 sa) { // IPv6接收 超时处理
    socklen_t sa_len = sizeof(sa);
    if (timeout == 0) { // 永久等待
        return recvfrom(fd, buffer, buffer_size, 0, (struct sockaddr*)&sa, &sa_len);
    }
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = timeout; // 超时时间 单位s
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    select(fd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
    if (FD_ISSET(fd, &rfds)) {
        return recvfrom(fd, buffer, buffer_size, 0, (struct sockaddr*)&sa, &sa_len);
    }
    return -1; // 接收超时
}

long ipv4_send_and_receive(char *ipv4_server_ip, int ipv4_server_port, char *send_buffer, long send_len, char *recv_buffer) { // IPv4下发送并接收
    struct sockaddr_in ipv4_server_addr;
    int ipv4_server_fd = socket(AF_INET, SOCK_DGRAM, 0); // 通讯描述符
    bzero(&ipv4_server_addr, sizeof(ipv4_server_addr)); // 清空为0x00
    ipv4_server_addr.sin_family = AF_INET;
    ipv4_server_addr.sin_port = htons(ipv4_server_port); // 目标端口
    ipv4_server_addr.sin_addr.s_addr = inet_addr(ipv4_server_ip); // 目标IP
    if (sendto(ipv4_server_fd, send_buffer, send_len, 0, (struct sockaddr*)&ipv4_server_addr, sizeof(ipv4_server_addr)) < 0) { // 发送缓冲区数据
        perror("[Shadowsocks Bootstrap] IPv4 UDP send failed");
    }
    long recv_len = ipv4_receive(ipv4_server_fd, recv_buffer, BUFFER_SIZE, TIMEOUT, ipv4_server_addr); // 接收数据到缓冲区
    close(ipv4_server_fd); // 关闭描述符
    return recv_len; // 返回接收长度
}

long ipv6_send_and_receive(char *ipv6_server_ip, int ipv6_server_port, char *send_buffer, long send_len, char *recv_buffer) { // IPv6下发送并接收
    struct sockaddr_in6 ipv6_server_addr;
    int ipv6_server_fd = socket(AF_INET6, SOCK_DGRAM, 0); // 通讯描述符
    bzero(&ipv6_server_addr, sizeof(ipv6_server_addr)); // 清空为0x00
    ipv6_server_addr.sin6_family = AF_INET6;
    ipv6_server_addr.sin6_port = htons(ipv6_server_port); // 目标端口
    inet_pton(AF_INET6, ipv6_server_ip, &ipv6_server_addr.sin6_addr); // 目标IP
    if (sendto(ipv6_server_fd, send_buffer, send_len, 0, (struct sockaddr*)&ipv6_server_addr, sizeof(ipv6_server_addr)) < 0) { // 发送缓冲区数据
        perror("[Shadowsocks Bootstrap] IPv6 UDP send failed");
    }
    long recv_len = ipv6_receive(ipv6_server_fd, recv_buffer, BUFFER_SIZE, TIMEOUT, ipv6_server_addr); // 接收数据到缓冲区
    close(ipv6_server_fd); // 关闭描述符
    return recv_len; // 返回接收长度
}

long send_and_receive(char *server_ip, int server_port, char *send_buffer, long send_len, char *recv_buffer) { // IPv4 / IPv6 双栈发送并接收
    if (is_ipv6_addr(server_ip)) { // IPv6 server
        return ipv6_send_and_receive(server_ip, server_port, send_buffer, send_len, recv_buffer);
    } else { // IPv4 (server_ip must be IPv4 or IPv6)
        return ipv4_send_and_receive(server_ip, server_port, send_buffer, send_len, recv_buffer);
    }
}

void ipv4_proxy(void *ipv4_info) { // 代理IPv4客户端
    ipv4_proxy_info *info = (ipv4_proxy_info*)ipv4_info;
    char *recv_buffer = (char*)malloc(BUFFER_SIZE); // 申请接收缓冲区内存
    long recv_len = send_and_receive(info->server_ip, info->server_port, info->buffer, info->len, recv_buffer); // 服务端交互
    if (recv_len < 0) { // 服务端超时
        printf("[Shadowsocks Bootstrap] UDP Proxy: server return timeout\n");
    } else {
        if (sendto(info->ipv4_client_fd, recv_buffer, recv_len, 0, (struct sockaddr*)&(info->ipv4_client_addr), sizeof(info->ipv4_client_addr)) < 0) { // 服务端数据返回给客户端
            perror("[Shadowsocks Bootstrap] IPv4 UDP return failed");
        } else {
            printf("[Shadowsocks Bootstrap] UDP Proxy: ↑ %ld bytes ↓ %ld bytes\n", info->len, recv_len);
        }
    }
    free(recv_buffer); // 释放接收缓冲区内存
    free(info->buffer); // 释放发送缓冲区内存
    free(ipv4_info); // 释放线程传参结构体
}

void ipv6_proxy(void *ipv6_info) { // 代理IPv6客户端
    ipv6_proxy_info *info = (ipv6_proxy_info*)ipv6_info;
    char *recv_buffer = (char*)malloc(BUFFER_SIZE); // 申请接收缓冲区内存
    long recv_len = send_and_receive(info->server_ip, info->server_port, info->buffer, info->len, recv_buffer); // 服务端交互
    if (recv_len < 0) { // 服务端超时
        printf("[Shadowsocks Bootstrap] Server return timeout\n");
    } else {
        if (sendto(info->ipv6_client_fd, recv_buffer, recv_len, 0, (struct sockaddr*)&(info->ipv6_client_addr), sizeof(info->ipv6_client_addr)) < 0) { // 服务端数据返回给客户端
            perror("[Shadowsocks Bootstrap] IPv6 UDP return failed");
        } else {
            printf("[Shadowsocks Bootstrap] UDP Proxy: ↑ %ld bytes ↓ %ld bytes\n", info->len, recv_len);
        }
    }
    free(recv_buffer); // 释放接收缓冲区内存
    free(info->buffer); // 释放发送缓冲区内存
    free(ipv6_info); // 释放线程传参结构体
}

void proxy(char *server_ip, int server_port, char *listen_ip, int listen_port) { // 代理UDP请求
    pthread_t tid;
    long recv_len;
    char recv_buffer[BUFFER_SIZE]; // 接收缓冲区
    int ipv4_client_fd = -1;
    int ipv6_client_fd = -1;
    struct sockaddr_in ipv4_client_addr;
    struct sockaddr_in6 ipv6_client_addr;
    socklen_t ipv4_client_addr_len = sizeof(ipv4_client_addr);
    socklen_t ipv6_client_addr_len = sizeof(ipv6_client_addr);
    int bind_error_flag = 0;
    int is_listen_ipv6 = is_ipv6_addr(listen_ip); // 判断监听地址是否为IPv6
    if (!is_listen_ipv6) { // IPv4客户端
        ipv4_client_fd = create_ipv4_udp_sock(listen_ip, listen_port); // 监听端口描述符
        if (ipv4_client_fd == -1) { // 端口监听失败
            bind_error_flag = 1;
        }
    } else { // IPv6客户端
        ipv6_client_fd = create_ipv6_udp_sock(listen_ip, listen_port); // 监听端口描述符
        if (ipv6_client_fd == -1) { // 端口监听失败
            bind_error_flag = 1;
        }
    }
    if (bind_error_flag) { // 端口被占用
        printf("[Shadowsocks Bootstrap] The UDP port seems to be occupied by the SIP003 plugin\n");
        printf("[Shadowsocks Bootstrap] WARNING: UDP communication of the agent will not work properly\n");
        return;
    }
    proxy_exit = 0; // 重置退出标识
    printf("[Shadowsocks Bootstrap] UDP Proxy: %s:%d -> %s:%d\n", listen_ip, listen_port, server_ip, server_port);
    for (;;) {
        if (!is_listen_ipv6) { // IPv4客户端
            recv_len = recvfrom(ipv4_client_fd, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr*)&ipv4_client_addr, &ipv4_client_addr_len);
            char *proxy_buffer = (char *) malloc(recv_len);
            memcpy(proxy_buffer, recv_buffer, recv_len); // 复制缓冲区数据
            ipv4_proxy_info *info = (ipv4_proxy_info *) malloc(sizeof(ipv4_proxy_info));
            info->server_ip = server_ip;
            info->server_port = server_port;
            info->ipv4_client_addr = ipv4_client_addr;
            info->ipv4_client_fd = ipv4_client_fd;
            info->buffer = proxy_buffer;
            info->len = recv_len;
            pthread_create(&tid, NULL, (void*)ipv4_proxy, (void*)info); // 新线程代理请求
        } else { // IPv6客户端
            recv_len = recvfrom(ipv6_client_fd, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr*)&ipv6_client_addr, &ipv6_client_addr_len);
            char *proxy_buffer = (char*)malloc(recv_len);
            memcpy(proxy_buffer, recv_buffer, recv_len); // 复制缓冲区数据
            ipv6_proxy_info *info = (ipv6_proxy_info*)malloc(sizeof(ipv6_proxy_info));
            info->server_ip = server_ip;
            info->server_port = server_port;
            info->ipv6_client_addr = ipv6_client_addr;
            info->ipv6_client_fd = ipv6_client_fd;
            info->buffer = proxy_buffer;
            info->len = recv_len;
            pthread_create(&tid, NULL, (void*)ipv6_proxy, (void*)info); // 新线程代理请求
        }
        if (proxy_exit) {
            break; // 退出代理
        }
    }
    sleep(TIMEOUT); // 等待线程结束
    if (!is_listen_ipv6) { // IPv4客户端
        close(ipv4_client_fd); // 关闭监听
    } else { // IPv6客户端
        close(ipv6_client_fd); // 关闭监听
    }
}
