#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "logger.h"
#include "dns.h"

char** init_dns_result();
char** add_dns_result(char **dns_result, char *str);
void free_dns_result(char **dns_result);
char** ipv4_dns_resolve(const char *domain);
char** ipv6_dns_resolve(const char *domain);

char** init_dns_result() { // 初始化DNS解析存储结构
    char **dns_result = (char**)malloc(sizeof(char*));
    dns_result[0] = NULL;
    return dns_result;
}

char** add_dns_result(char **dns_result, char *str) { // 添加DNS解析记录
    int num = 0;
    while(dns_result[num++] != NULL); // 获取原存储个数
    dns_result = (char**)realloc(dns_result, sizeof(char*) * (num + 1));
    dns_result[num - 1] = strcpy((char*)malloc(strlen(str) + 1), str);
    dns_result[num] = NULL; // 结束标志
    return dns_result;
}

void free_dns_result(char **dns_result) { // 释放DNS解析结果
    int num = 0;
    while(dns_result[num] != NULL) { // 逐个释放
        free(dns_result[num++]);
    }
}

char** ipv4_dns_resolve(const char *domain) { // DNS解析IPv4地址
    char **result = init_dns_result();
    char ip_str[16]; // IPv4地址字符串 (3 * 4 + 1 * 3 + 1)
    struct sockaddr_in *ipv4_addr;
    struct addrinfo *answer, hint, *p;
    bzero(&hint, sizeof(hint)); // 清空为0x00
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    int ret = getaddrinfo(domain, NULL, &hint, &answer); // 发起解析
    if (ret != 0) { // 解析失败
        log_debug("IPv4 DNS resolve `%s`: %s", domain, gai_strerror(ret));
        return result; // 返回空数据
    }
    for (p = answer; p != NULL; p = p->ai_next) { // 遍历解析结果
        ipv4_addr = (struct sockaddr_in*)(p->ai_addr); // 获取IPv4地址
        inet_ntop(AF_INET, &ipv4_addr->sin_addr, ip_str, sizeof(ip_str)); // 转化为字符串形式
        result = add_dns_result(result, ip_str);
    }
    freeaddrinfo(answer); // 释放解析结果
    return result;
}

char** ipv6_dns_resolve(const char *domain) { // DNS解析IPv6地址
    char **result = init_dns_result();
    char ip_str[40]; // IPv6地址字符串 (4 * 8 + 1 * 7 + 1)
    struct sockaddr_in6 *ipv6_addr;
    struct addrinfo *answer, hint, *p;
    bzero(&hint, sizeof(hint)); // 清空为0x00
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_STREAM;
    int ret = getaddrinfo(domain, NULL, &hint, &answer); // 发起解析
    if (ret != 0) { // 解析失败
        log_debug("IPv6 DNS resolve `%s`: %s", domain, gai_strerror(ret));
        return result; // 返回空数据
    }
    for (p = answer; p != NULL; p = p->ai_next) { // 遍历解析结果
        ipv6_addr = (struct sockaddr_in6*)(p->ai_addr); // 获取IPv6地址
        inet_ntop(AF_INET6, &ipv6_addr->sin6_addr, ip_str, sizeof(ip_str)); // 转化为字符串形式
        result = add_dns_result(result, ip_str);
    }
    freeaddrinfo(answer); // 释放解析结果
    return result;
}

char* dns_resolve(const char *domain) { // DNS解析 返回首个IP地址 IPv4优先
    int num = 0;
    char **result = ipv4_dns_resolve(domain); // IPv4解析
    while(result[num++] != NULL); // num - 1 为IPv4地址数
    if (num - 1 != 0) { // 存在IPv4解析
        char *tmp = strcpy((char*)malloc(strlen(result[0]) + 1), result[0]);
        free_dns_result(result); // 释放IPv4结果
        return tmp; // 返回首个IPv4地址
    }
    free_dns_result(result); // 释放IPv4结果
    result = ipv6_dns_resolve(domain);
    num = 0;
    while(result[num++] != NULL); // num - 1 为IPv6地址数
    if (num - 1 == 0) { // 无IPv6解析
        free_dns_result(result); // 释放IPv6结果
        return NULL;
    }
    char *tmp = strcpy((char*)malloc(strlen(result[0]) + 1), result[0]);
    free_dns_result(result); // 释放IPv6结果
    return tmp; // 返回首个IPv6地址
}
