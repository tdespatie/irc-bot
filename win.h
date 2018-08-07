#ifndef IRC_SOCKET_H
#define IRC_SOCKET_H

#include <stdlib.h>
#include <winsock2.h>

#define ERR -1
#define MATCH 0
#define SUCCESS 0

SOCKET get_socket(const char* host, const char* port);
int sck_send(SOCKET socket, const char* data, size_t size);
int sck_sendf(SOCKET socket, const char* fmt, ...);
int sck_recv(SOCKET socket, char* buffer, size_t size);

#endif
