/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#include "UdpTransport.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../util/ndn_memory.h"

ndn_Error ndn_UdpTransport_connect(struct ndn_UdpTransport *self, char *host, unsigned short port)
{
  if (self->socketDescriptor >= 0) {
    close(self->socketDescriptor);
    self->socketDescriptor = -1;
  }
  
	struct addrinfo hints;
	ndn_memset((unsigned char *)&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

  char portString[10];
  sprintf(portString, "%d", port);
  
	struct addrinfo *serverInfo;
	if (getaddrinfo(host, portString, &hints, &serverInfo) != 0)
		return NDN_ERROR_UdpTransport_error_in_getaddrinfo;

	// loop through all the results and connect to the first we can
	struct addrinfo *p;
  int socketDescriptor;
	for(p = serverInfo; p != NULL; p = p->ai_next) {
		if ((socketDescriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (connect(socketDescriptor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socketDescriptor);
			continue;
		}

		break;
	}

	if (p == NULL) {
    freeaddrinfo(serverInfo);
		return NDN_ERROR_UdpTransport_cannot_connect_to_socket;
  }

	freeaddrinfo(serverInfo);
  self->socketDescriptor = socketDescriptor;

  return 0;
}

ndn_Error ndn_UdpTransport_send(struct ndn_UdpTransport *self, unsigned char *data, unsigned int dataLength)
{
  if (self->socketDescriptor < 0)
    return NDN_ERROR_UdpTransport_socket_is_not_open;
  
  int nBytes;
  while (1) {
    if ((nBytes = send(self->socketDescriptor, data, dataLength, 0)) < 0)
      return NDN_ERROR_UdpTransport_error_in_send;
    if (nBytes >= dataLength)
      break;
    
    // Send more.
    dataLength -= nBytes;
  }

  return 0;  
}

ndn_Error ndn_UdpTransport_receive
(struct ndn_UdpTransport *self, unsigned char *buffer, unsigned int bufferLength, unsigned int *nBytesOut)
{
  if (self->socketDescriptor < 0)
    return NDN_ERROR_UdpTransport_socket_is_not_open;

  int nBytes;  
	if ((nBytes = recv(self->socketDescriptor, buffer, bufferLength, 0)) == -1)
    return NDN_ERROR_UdpTransport_error_in_recv;

  *nBytesOut = (unsigned int)nBytes;
  
	return 0;  
}
