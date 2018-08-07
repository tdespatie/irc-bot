#define _CRT_SECURE_NO_WARNINGS
#include <sys/types.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "win.h"

#pragma comment(lib, "ws2_32.lib")


SOCKET get_socket(const char* host, const char* port)
{
   WSADATA wsaData;
   SOCKET ConnectSocket = INVALID_SOCKET;
   int rc;
   struct addrinfo hints, *res;

   rc = WSAStartup(MAKEWORD(2,2), &wsaData);
   if (rc != SUCCESS)
   {
     fprintf(stderr, "WSAStartup error");
     return ERR;
   }

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = IPPROTO_TCP;

   rc = getaddrinfo(host, port, &hints, &res);

   if (rc != SUCCESS)
   {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
      WSACleanup();
      return ERR;
   }

   ConnectSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if ( ConnectSocket == INVALID_SOCKET)
   {
      fprintf(stderr, "Couldn't get socket.\n");
      WSACleanup();
      goto error;
   }

   if ( connect(ConnectSocket, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR )
   {
      closesocket(ConnectSocket);
      fprintf(stderr, "Couldn't connect.\n");
      goto error;
   }

   freeaddrinfo(res);
   return ConnectSocket;

error:
   freeaddrinfo(res);
   return ERR;

}

int sck_send(SOCKET s, const char* data, size_t size)
{
   size_t written = 0;
   int rc;

   while ( written < size )
   {
      rc = send(s, data + written, size - written, 0);
      if ( rc == SOCKET_ERROR ) {
         closesocket(s);
         WSACleanup();
         return ERR;
      }

      written += rc;
	  printf("SENT: %s", data);
   }

   return written;
}

int sck_sendf(SOCKET s, const char *fmt, ...)
{
   char send_buf[512];
   int send_len;
   va_list args;

   if (strlen(fmt) != MATCH )
   {
      // Format the data
      va_start(args, fmt);
      send_len = vsnprintf(send_buf, sizeof (send_buf), fmt, args);
      va_end(args);

      // Clamp the chunk
      if (send_len > 512)
         send_len = 512;

      if (sck_send( s, send_buf, send_len ) <= 0)
         return ERR;
      return send_len;
   }
   return SUCCESS;
}

int sck_recv(SOCKET s, char* buffer, size_t size)
{
   int rc;
   
   rc = recv(s, buffer, size, 0);
   if ( rc <= 0 )
      return ERR;

   return rc;
}