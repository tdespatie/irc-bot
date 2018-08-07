#include "win.h"
#include "irc.h"

int main(int argc, char **argv)
{
   const char* IP_ADDR = "38.229.70.22";
   const char* PORT = "6667";
   const char* NICKNAME = "I-Am-A-Bot";
   const char* CHANNEL = "#test-channel";
   const char* USERNAME = "IAMBOT";
   const char* REALNAME = "Bot written in C";

   irc_t irc;

   if (argc > 1) {
	   if (strncmp("-v", argv[1], 2) == 0) {
		   irc.verbose = 1;
		   printf("Verbose mode.\n");
	   }
   }

   if ( irc_connect(&irc, IP_ADDR, PORT) < 0 ) // IP Address and Port Number
   {
      fprintf(stderr, "Connection failed.\n");
      goto exit_err;
   }

  // irc_set_output(&irc, "/dev/stdout"); // Only for linux

   if ( irc_login(&irc, NICKNAME, USERNAME, REALNAME) < 0 ) // Nickname
   {
      fprintf(stderr, "Couldn't log in.\n");
      goto exit_err;
   }

   if ( irc_join_channel(&irc, CHANNEL) < 0 ) // Channel to join once connected.
   {
      fprintf(stderr, "Couldn't join channel.\n");
      goto exit_err;
   }
   
   while ( irc_handle_data(&irc) >= 0 ); // Enter an infinite loop to listen to requests

   irc_close(&irc);
   return 0;

exit_err:
   irc_close(&irc);
   exit(1); 
}

