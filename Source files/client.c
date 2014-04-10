#include "main.h"

int global_sd;
char* usern;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startClient
--
-- DATE: March 10, 2014
--
-- REVISIONS:
--
-- DESIGNER: Robin Hsieh
--
-- PROGRAMMER: Robin Hsieh
--
-- INTERFACE: startClient(char* username - username of client
--                      , char*     host - ip address of server
--                      , int       port - port of server
--                       )
--
-- RETURNS: void
--
-- NOTES: A client is started with username, server port and ip given via command line arguments. User
--        input is read through stdin and sent to the server. A seperate process is made to read data
--        from the server.
--
------------------------------------------------------------------------------------------------------------------*/
void startClient(char* username, char* host, int port)
{
    int     n,
            bytes_to_read,
            sd,
            lengthOfServerName;

    char   *bp,
            rbuf[BUFLEN],
            sbuf[BUFLEN],
            userText[BUFLEN],
          **pptr,
            serverName[BUFLEN],
            ctrlCharacter1,
            ctrlCharacter2,
            ctrlCharacter3;

    char    str[16];

    FInfo *fi = mmap(NULL, sizeof(FInfo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    fi->log = 0;

    struct hostent	*hp;
    struct sockaddr_in server;

    usern = username;

    signal(SIGINT, catch_int);

    // Create the socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Cannot create socket");
        exit(1);
    }

    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL)
    {
        fprintf(stderr, "Unknown server address\n");
        exit(1);
    }
    bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

    // Connecting to the server
    if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        fprintf(stderr, "Can't connect to server\n");
        perror("connect");
        exit(1);
    }

    global_sd = sd;

    sbuf[0] = '\0';
    sprintf(sbuf, "%c %c %zu %s %c", SYN, 0x02, strlen(username), username, EOT);

    send(sd, sbuf, BUFLEN, 0);

    // read the server name
    memset(rbuf, 0, BUFLEN);
    read(sd, rbuf, BUFLEN);

    sscanf(rbuf, "%c %c %d %s %c", &ctrlCharacter1, &ctrlCharacter2, &lengthOfServerName, serverName, &ctrlCharacter3);

    printf("Connected:    Server Name: %s\n", serverName);
    pptr = hp->h_addr_list;
    printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));

    switch(fork())
    {
    case -1:
        perror("fork call");
        exit(1);
    case 0:
        while(1)
        {
            bp = rbuf;
            bytes_to_read = BUFLEN;
            n = 0;

            while((n = recv(sd, bp, bytes_to_read, 0)) < BUFLEN)
            {
                bp += n;
                bytes_to_read -= n;
            }

            /* handle server termination signal */
            if(!strcmp(rbuf, "terminate") || !strcmp(rbuf, "you have been kicked"))
            {
                printf("Server ");
                fflush(stdout);
                kill(0, SIGINT);
            }

            printf("%s", rbuf);
            /* log chat in text file if activated */
            if(fi->log)
            {
                fi->fp = fopen("log.txt", "a+");
                fprintf(fi->fp, rbuf);
                fclose(fi->fp);
            }

            fflush(stdout);
        }

    default:
        while(1)
        {
            memset(sbuf, 0, BUFLEN);
            memset(userText, 0, BUFLEN);

            fgets (userText, BUFLEN, stdin);

            strcpy(sbuf, username);
            /* displays list of commands */
            if(!strcmp(userText, "/help\n"))
            {
                printf("User commands\n");
                printf("==========================================================\n");
                printf("/users - displays list of current users and status\n");
                printf("/msg <username> - send a message to specific user\n");
                printf("/quit - leave the chat\n\n");
                printf("Admin commands\n");
                printf("==========================================================\n");
                printf("/kick <username> - removes a user from the chat\n");
                printf("/promote <username> - give a user admin privileges\n");
                printf("/promote <username> - removes admin privileges from a user\n");
                printf("\n");
            }
            /* user quits the application */
            if(!strcmp(userText, "/quit\n"))
            {
                strcat(sbuf, " has left the chat\n");
                send (sd, sbuf, BUFLEN, 0);

                if(fi->log)
                    fclose(fi->fp);

                if(munmap(fi, sizeof(FInfo)) == -1)
                    perror("un-mapping");

                kill(0, SIGINT);
            }
            /* toggles logging chat */
            if(!strcmp(userText, "/log\n"))
            {
                if(!fi->log)
                    fi->log = 1;
                else
                    fi->log = 0;
            }
            else
            {
                strcat(sbuf, ": ");
                strcat(sbuf, userText);
                // Transmit data through the socket
                if(fi->log)
                {
                    fi->fp = fopen("log.txt", "a+");
                    fprintf(fi->fp, sbuf);
                    fclose(fi->fp);
                }
                send (sd, sbuf, BUFLEN, 0);
            }

        }
    }

    close(sd);
    return;
}
/* kill process upon termination */
void catch_int(int signo)
{
    if(signo == SIGINT)
    {
        char sbuf[BUFLEN];
        strcpy(sbuf, usern);
        strcat(sbuf, " has left the chat\n");
        send (global_sd, sbuf, BUFLEN, 0);
        close(global_sd);
        kill(getpid(), SIGTERM);
    }
}
