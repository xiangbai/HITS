#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "socket_utils.h"
#include "general_utils.h"
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>

struct timeval rcv_timeout;

int connect_socket(char *host_string, char *port_string, FILE *stream)
{
	int socket_id = -1;
	
	// make criteria
	struct addrinfo addr_criteria;
	memset(&addr_criteria, 0, sizeof(addr_criteria));
	addr_criteria.ai_family = AF_UNSPEC;		// any ip version
	addr_criteria.ai_socktype = SOCK_STREAM;	// sockets only
	addr_criteria.ai_protocol = IPPROTO_TCP;	// tcp only
   
	// get list of addresses
	struct addrinfo *address_list;
	int val = getaddrinfo(host_string, port_string, &addr_criteria, &address_list);
	
	// if error, notify user and return -1
	if (val)
	{
		printf("Error w/ host [%s]\n", host_string);
		report_error("error in getaddrinfo()");
		
		return -1;
   	}
 
	// traverse linked list
	struct addrinfo *current;
	for (current = address_list; current; current = current->ai_next)
	{
		// create socket
		socket_id = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
		if (socket_id < 0)
			continue;
		
		// set timeout value for receiving data from pages
		rcv_timeout.tv_sec = 10;
		rcv_timeout.tv_usec = 0;
		// set timeout on receiving data
		if (setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, 
				(char*)&rcv_timeout, sizeof(rcv_timeout)) < 0)
		{
			// return null if unable to set timeout
			report_error("unable to set socket timeout");
			return NULL;
		}
		
#if defined(SO_NOSIGPIPE)
		int val = 1;
		setsockopt(socket_id, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(int));
#elif !defined(MSG_NOSIGNAL)
		report_error("This program requires systems to have definitions for SO_NOSIGPIPE or MSG_NOSIGNAL");
#endif
		
		// connect to server (break if connection is successful)
		int connectval = connect(socket_id, current->ai_addr, current->ai_addrlen);
		if (connectval <= 0)
			break;
       
		printf("ERROR: %s\n", strerror(errno));
		
		// if not connected, close socket and mark as non-functional
		close(socket_id);
		socket_id = -1;
	}
    
	// free memory
	freeaddrinfo(address_list);
    
	return socket_id;
}

void print_address(const struct sockaddr *address, FILE *stream)
{
	void *numeric_address;
	in_port_t port_number;
    
	// set numeric address and port
	switch(address->sa_family)
	{
        case AF_INET:
            numeric_address = &((struct sockaddr_in *) address)->sin_addr;
            port_number = ntohs(((struct sockaddr_in *) address)->sin_port);
            break;
        case AF_INET6:
            numeric_address = &((struct sockaddr_in6 *) address)->sin6_addr;
            port_number = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
            break;
        default:
            fputs("[ERROR on print_address()]", stream);
            return;
	}
	
	// convert to printable and display
	char address_buffer[INET6_ADDRSTRLEN];
	if (inet_ntop(address->sa_family, numeric_address, address_buffer, sizeof(address_buffer)))
	{
		fprintf(stream, "%s ", address_buffer);
		if (port_number)
			fprintf(stream, "-%u", port_number);
	}
	else
		fputs("[invalid address]", stream);
	
}
