#ifndef __IRC_H
#define __IRC_H

#include <stdio.h>
#include <winsock2.h>

#define ERR -1
#define MATCH 0
#define SUCCESS 0

typedef struct
{
   SOCKET s;
   FILE *file;
   char channel[256];
   char servbuf[512];
   int bufptr;
   int verbose;
} irc_t; 

void irc_close(irc_t *);
int irc_connect(irc_t *, const char*, const char*);
int irc_login(irc_t *irc, const char*, const char*, const char*);
int irc_join_channel(irc_t *, const char*);
int irc_leave_channel(irc_t *);
int irc_handle_data(irc_t *);
int irc_set_output(irc_t *, const char*);
int irc_parse_action(irc_t *);
int irc_log_message(irc_t *, const char*, const char*, int);
int irc_reply_message(irc_t *, char*, char*);

// IRC Protocol
int irc_pong(SOCKET, const char *);
int irc_reg(SOCKET, const char *, const char *, const char *);
int irc_join(SOCKET, const char *);
int irc_part(SOCKET, const char *);
int irc_nick(SOCKET, const char *);
int irc_quit(SOCKET);
int irc_topic(SOCKET, const char *, const char *);
int irc_action(SOCKET, const char *, const char *);
int irc_msg(SOCKET, const char *, const char *);

#endif
