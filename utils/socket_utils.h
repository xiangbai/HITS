#include <netinet/in.h>

int connect_socket(char *host_string, char *port_string, FILE *stream);
void print_address(const struct sockaddr *address, FILE *stream);