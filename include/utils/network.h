#ifndef _NETWORK_H_
#define _NETWORK_H_

extern int PROXY_EXIT;

int is_ip_addr(char *address);
int get_available_port(unsigned short range_start, unsigned short range_end);
void proxy(char *server_ip, int server_port, char *listen_ip, int listen_port);

#endif
