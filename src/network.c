#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "network.h"

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
