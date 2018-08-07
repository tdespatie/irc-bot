#define _CRT_SECURE_NO_WARNINGS
#include "win.h"
#include "irc.h"
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

// Connect to the IRC server at specified IP ADDRESS and PORT
int irc_connect(irc_t *irc, const char* server, const char* port)
{
	if ((irc->s = get_socket(server, port)) < 0) return ERR;

	irc->bufptr = 0; // Clear the buffer

	return SUCCESS;
}

// Login/Provide IRC server with information about the bot
int irc_login(irc_t *irc, const char* nick, const char* username, const char* realname)
{
	return irc_reg(irc->s, nick, username, realname); // GECOS field (Provides server with your Real Name/Purpose)
}

// Join channel specified
int irc_join_channel(irc_t *irc, const char* channel)
{
	strncpy(irc->channel, channel, 254); // Copy 
	irc->channel[254] = '\0'; // EOF Character
	return irc_join(irc->s, channel);
}

// Leave the current channel
int irc_leave_channel(irc_t *irc)
{
	return irc_part(irc->s, irc->channel);
}

int irc_handle_data(irc_t *irc)
{
	char tempbuffer[512];
	int rc, i;

	rc = sck_recv(irc->s, tempbuffer, sizeof(tempbuffer) - 2);

	if (rc <= 0)
	{
		fprintf(stderr, "An error has occurred\n");
		return ERR;
	}

	tempbuffer[rc] = '\0';

	for (i = 0; i < rc; ++i)
	{
		switch (tempbuffer[i])
		{
		case '\r': // Check for carriage return
		case '\n': // Check for line feed
		{
			irc->servbuf[irc->bufptr] = '\0'; // End the line
			irc->bufptr = 0; // Reset the buffer

			if (irc_parse_action(irc) < 0) // Parse the line, could be a command
				return ERR;

			break;
		}

		default: // Advance the buffer until end of line
		{
			irc->servbuf[irc->bufptr] = tempbuffer[i];
			if (irc->bufptr >= (sizeof(irc->servbuf) - 1))
				return ERR; // Buffer overflow

			irc->bufptr++; // Advance the buffer
		}
		}
	}
	return SUCCESS;
}

int irc_parse_action(irc_t *irc)
{
	char irc_nick[128];
	char irc_msg[512];

	static char concat_string[258];

	if (strlen(concat_string) == 0) // Concatenate '= ' with channel name to detect it in message later
		snprintf(concat_string, 258, "= %s", irc->channel);

	if (strncmp(irc->servbuf, "PING :", 6) == MATCH)
	{
		if (irc->verbose == 1) printf("EVENT: RECIEVED PING\r\n");
		return irc_pong(irc->s, &irc->servbuf[6]);
	}
	else if (strncmp(irc->servbuf, "NOTICE AUTH :", 13) == MATCH)
	{
		if (irc->verbose == 1) printf("EVENT: NOTICE AUTH : %s\r\n", irc->servbuf);
		return SUCCESS; // Parsed properly
	}
	else if (strncmp(irc->servbuf, "ERROR :", 7) == MATCH)
	{
		if (irc->verbose == 1) printf("%s\r\n", irc->servbuf);
		return SUCCESS; // Parsed properly
	}
	else if (strstr(irc->servbuf, concat_string) != NULL)
	{
		printf("EVENT: Joined channel %s\r\n", irc->channel);
		return SUCCESS;
	}

	// Parses IRC message that pulls out nick and message
	else
	{
		char *ptr;
		int privmsg = 0;
		int publicmsg = 0;

		*irc_nick = '\0';
		*irc_msg = '\0';

		// Checks if we have non-message string
		if (strchr(irc->servbuf, 1) != NULL)
			return 0;

		if (irc->servbuf[0] == ':') // Valid server message
		{

			if (irc->verbose)
				printf("RECEIVED: %s\r\n", irc->servbuf);
			
			ptr = strtok(irc->servbuf, "!"); // Check if message from users

			if (ptr == NULL)
			{
				printf("ptr == NULL\r\n");
				return 0;
			}
			else
			{
				strncpy(irc_nick, &ptr[1], 127); // Store nickname of messager
				irc_nick[127] = '\0';
			}

			while ((ptr = strtok(NULL, " ")) != NULL) {
				// Check for PRIVMSG command from server
				if (strcmp(ptr, "PRIVMSG") == MATCH) 
				{
					if ((ptr = strtok(NULL, " ")) != NULL && (strchr(ptr, '#') == NULL)) 
						privmsg = 1; // Message was sent to us directly through private message
					else 
						publicmsg = 1; // Message was sent to us in the chat channel.
					break; // We know this only contains a PRIVMSG
				}
			}



			if (publicmsg || privmsg)
			{
				ptr = strtok(NULL, ":"); // Advance to the message

				if (ptr != NULL)
				{
					strncpy(irc_msg, ptr, 511);
					irc_msg[511] = '\0';
					irc_log_message(irc, irc_nick, irc_msg, privmsg);

					if (irc_reply_message(irc, irc_nick, irc_msg) < 0)
						return ERR;
				}

			}

			if (privmsg)
			{
				// This is a private message
			}

		}
	}
	return SUCCESS;
}

int irc_set_output(irc_t *irc, const char* file)
{
	irc->file = fopen(file, "w");
	if (irc->file == NULL)
		return ERR;
	return SUCCESS;
}

int irc_reply_message(irc_t *irc, char *irc_nick, char *msg)
{
	if (*msg != '!') 
		return SUCCESS; // Checks if someone calls on the bot.

	char *command;
	char *arg;
	
	command = strtok(&msg[1], " "); // Get command
	arg = strtok(NULL, "");

	if (arg != NULL)
		while (*arg == ' ') // Advance to get argument
			arg++;

	if (command == NULL) 
		return SUCCESS; // Didn't find a command

	if (strcmp(command, "ping") == MATCH)
	{
		if (irc_msg(irc->s, irc->channel, "pong") < 0) // Send pong
			return ERR;
	}
	else if (strcmp(command, "quit") == MATCH)
	{
		if (irc_msg(irc->s, irc->channel, "Okay, I'm leaving.") < 0)
			return ERR;

		if (irc_quit(irc->s) < 0)
			return ERR;

		// Has to be less than 0 to break the loops but not -1 because that indicates an error
		return -2; 
	}
	else if (strcmp(command, "google") == MATCH)
	{
		char mesg[512];

		char t_nick[128];
		char t_link[256];
		char link[256] = { 0 };

		char *t_arg = strtok(arg, " ");
		if (t_arg)
		{
			strncpy(t_nick, t_arg, 127);
			t_nick[127] = '\0';
		}
		else
			return SUCCESS;

		t_arg = strtok(NULL, "");
		if (t_arg)
		{
			while (*t_arg == ' ')
				t_arg++;

			strncpy(t_link, t_arg, 255);
			t_link[255] = '\0';
		}
		else
			return SUCCESS;

		t_arg = strtok(t_link, " ");
		while (t_arg)
		{
			strncpy(&link[strlen(link)], t_arg, 254 - strlen(link));

			t_arg = strtok(NULL, " ");
			if (!t_arg)
				break;

			strncpy(&link[strlen(link)], "%20", 254 - strlen(link));
		}


		sprintf_s(mesg, 511, "%s: http://lmgtfy.com/?q=%s", t_nick, link);
		mesg[511] = '\0';
		if (irc_msg(irc->s, irc->channel, mesg) < 0)
			return ERR;
	}

	return SUCCESS;
}

int irc_log_message(irc_t *irc, const char* nick, const char* message, int privmsg)
{
	char timestring[128];

	_tzset();
	_strtime_s(timestring, 127);
	timestring[127] = '\0';

	if (privmsg) {
		fprintf(stdout, "PRIVATE - [%s] <%s> %s\r\n", timestring, nick, message);
	}
	else {
		fprintf(stdout, "%s - [%s] <%s> %s\r\n", irc->channel, timestring, nick, message);
	}

	fflush(stdout);
	return SUCCESS;
}

void irc_close(irc_t *irc)
{
	closesocket(irc->s);
	// fclose(irc->file);
}


// irc_pong: For answering pong requests...
int irc_pong(SOCKET s, const char *data)
{
	return sck_sendf(s, "PONG\r\n", data);
}

// irc_reg: For registering upon login
int irc_reg(SOCKET s, const char *nick, const char *username, const char *fullname)
{
	return sck_sendf(s, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

// irc_join: For joining a channel
int irc_join(SOCKET s, const char *data)
{
	return sck_sendf(s, "JOIN %s\r\n", data);

}

// irc_part: For leaving a channel
int irc_part(SOCKET s, const char *data)
{
	return sck_sendf(s, "PART %s\r\n", data);

}

// irc_nick: For changing your nick
int irc_nick(SOCKET s, const char *data)
{
	return sck_sendf(s, "NICK %s\r\n", data);

}

// irc_quit: For quitting IRC
int irc_quit(SOCKET s)
{
	return sck_sendf(s, "QUIT\r\n");
}

// irc_topic: For setting or removing a topic
int irc_topic(SOCKET s, const char *channel, const char *data)
{
	return sck_sendf(s, "TOPIC %s :%s\r\n", channel, data);
}

// irc_action: For executing an action (.e.g /me is hungry)
int irc_action(SOCKET s, const char *channel, const char *data)
{
	return sck_sendf(s, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
}

// irc_msg: For sending a channel message or a query
int irc_msg(SOCKET s, const char *channel, const char *data)
{
	return sck_sendf(s, "PRIVMSG %s :%s\r\n", channel, data);
}

