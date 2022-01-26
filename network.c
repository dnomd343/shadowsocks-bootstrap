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

int check_port_available(unsigned short port) { // test a port is available or not
    struct sockaddr_in server_addr; 
    memset(&server_addr, 0, sizeof(server_addr)); // struct init
    server_addr.sin_family = AF_INET; // set as IP communication
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // allow any connection
    server_addr.sin_port = htons(port); // set telnet port
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0); // socket created
    if (server_sockfd < 0) { // create failed
        return 0;
    }
    if (bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) { // bind failed
        return 0;
    }
    if (close(server_sockfd) != 0) { // close failed
        return 0;
    }
    return 1; // port available
}

int get_available_port(unsigned short range_start, unsigned short range_end) { // get a available port
    unsigned short port;
    for (;;) { // wait until a available port in range
        port = get_random_num(range_start, range_end); // get a random port in range
        if (check_port_available(port)) { // port available
            return (int)port;
        }
    }
}
