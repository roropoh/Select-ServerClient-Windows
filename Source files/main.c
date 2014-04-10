/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.c - This application is an application to start a server, or client to act as a chat program.
--                          The server will listen for clients, receive messages, and send received messages to
--                          all other clients that are connected to the server. The client will connect to a server,
--                          and be able to send messages to the server.
--
-- PROGRAM: lmsn
--
-- FUNCTIONS:
--      int main (int argc , char *argv[])
--      void exit_message()
--
-- DATE: March 10, 2014
--
-- DESIGNER: Robin Hsieh A00657820
--
-- PROGRAMMER: Robin Hsieh A00657820
--
-- NOTES:
-- This application is an application to start a server, or client to act as a chat program.
-- The server will listen for clients, receive messages, and send received messages to
-- all other clients that are connected to the server. The client will connect to a server,
-- and be able to send messages to the server.
--
----------------------------------------------------------------------------------------------------------------------*/

#include "main.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: main
--
-- DATE: March 10, 2014
--
-- REVISIONS:
--
-- DESIGNER: Robin Hsieh
--
-- PROGRAMMER: Robin Hsieh
--
-- INTERFACE: int main (int argc , char *argv[])
--              int argc:       The number of the arguments put in from the command line.
--              char *argv[]:   The array of char* of the arguments
--
-- RETURNS: int
--              Returns an int when program terminates.
--
-- NOTES:
-- This function is to start the server program to be able to allow clients to connect, and act as a echo server
-- that will send message from 1 client, to the rest of other clients connected. This program can also star the
-- client program that will be able to connect to a server, and send messages to that server.
--
------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char username[128];
    char ip[128];
    int port;

    if(argc == 3)
    {
        if(strcmp(argv[1], "server") == 0)
        {
            port = atoi(argv[2]);
            startServer(port);
        }
        else
        {
            exit_message();
        }
    }
    else if(argc == 5)
    {
        if(strcmp(argv[1], "client") == 0)
        {
            strcpy(username, argv[2]);
            strcpy(ip, argv[3]);
            port = atoi(argv[4]);

            startClient(username, ip, port);
        }
        else
        {
            exit_message();
        }
    }
    else
    {
        exit_message();
    }

    return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: exit_message
--
-- DATE: January 13, 2014
--
-- REVISIONS:  March 10, 2014
--             (Robin) - Changed Usage message
--
-- DESIGNER: Robin Hsieh
--
-- PROGRAMMER: Robin Hsieh
--
-- INTERFACE: void exit_message()
--
-- RETURNS: Void
--
-- NOTES:
-- This function is to print the error message when user inputs wrong arguments into the command line.
--
------------------------------------------------------------------------------------------------------------------*/
void exit_message()
{
    fprintf(stderr, "Usage:\t./lmsn client username ip port\n");
    fprintf(stderr, "Or:\t./lmsn server port\n");
    exit(1);
}
