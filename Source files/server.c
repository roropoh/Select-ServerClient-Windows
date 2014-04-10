#include "main.h"

CList client_list[MAX_CLIENTS];
int numClients;
int client[FD_SETSIZE];

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startServer
--
-- DATE: March 14, 2014
--
-- REVISIONS: March 16 - added printing users list and users quitting
--            March 17 - added messaging specific users
--            March 20 - added role based users
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: void startServer(
--                             int port - port number for server
--                             )
--
-- RETURNS: void
--
-- NOTES: The server will run in a forever loop checking and updating connections using select()
--        Special commands such as kicking a user and promoting a user are also handled by the server
--
------------------------------------------------------------------------------------------------------------------*/
void startServer(int port)
{
    char serverName[BUFLEN];
    printf("Please enter a server name: \n");
    fgets(serverName, BUFLEN, stdin);

    signal(SIGINT, catch_int_server);

    int  listen_sd, new_sd, sockfd,
         maxfd,
         maxi,
         bytes_to_read,
         i, j, k,
         promotion,
         nready,
         lengthOfClientName;

    char sbuf[BUFLEN],
        *bp,
         buf[BUFLEN],
         msgUser[30],
         ctrlCharacter1,
         ctrlCharacter2,
         ctrlCharacter3,
         clientName[BUFLEN];

    socklen_t client_len;

    struct sockaddr_in server, client_addr;

    ssize_t n;
    fd_set rset, allset;
    printf("Server name: %s", serverName);
    printf("Server listening on port: %d\n", port);

    listen_socket(&listen_sd, &server, port);

    if (bind(listen_sd, (struct sockaddr *)&server, sizeof(server)) == -1)
        SystemFatal("bind error");

    listen(listen_sd, LISTENQ);

    maxfd = listen_sd;
    maxi = -1;
    numClients = 0;

    for(i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listen_sd, &allset);
    /* set sockets in client struct to -1 */
    for(i = 0; i < MAX_CLIENTS; i++)
        client_list[i].socket = -1;


    while(1)
    {
        memset(buf, 0, BUFLEN);
        rset = allset;               // structure assignment
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_sd, &rset)) // new client connection
        {
            client_len = sizeof(client_addr);
            if ((new_sd = accept(listen_sd, (struct sockaddr *) &client_addr, &client_len)) == -1)
                SystemFatal("accept error");

            bp = buf;
            read(new_sd, bp, BUFLEN);
            sscanf(bp, "%c %c %d %s %c", &ctrlCharacter1, &ctrlCharacter2, &lengthOfClientName, clientName, &ctrlCharacter3);



            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = new_sd;    // save descriptor

                    printf("%s has connected with the remote address of %s\n", clientName, inet_ntoa(client_addr.sin_addr));
                    // send server name
                    memset(sbuf, 0, BUFLEN);
                    sprintf(sbuf, "%c %c %zu %s %c", SYN, 0x03, strlen(serverName), serverName, EOT);
                    write(client[i], sbuf, BUFLEN);

                    for(j = 0; j < MAX_CLIENTS; j++)
                    {
                        if(client_list[j].socket == -1)
                        {
                            if(numClients == 0)
                                addClient(j, i, 'a', clientName, inet_ntoa(client_addr.sin_addr));
                            else
                                addClient(j, i, 'n', clientName, inet_ntoa(client_addr.sin_addr));
                            break;
                        }
                    }

                    break;
                }
            }
            if (i == FD_SETSIZE)
            {
                printf ("Too many clients\n");
                exit(1);
            }

            memset(sbuf, 0, BUFLEN);
            strcat(sbuf, inet_ntoa(client_addr.sin_addr));
            strcat(sbuf, " ");
            strcat(sbuf, clientName);
            strcat(sbuf, " joined the chat channel.\n");

            /* notify other users that a new user has joined the chat */
            for(j = 0; j <= maxfd; j++)
            {
                if(client[j] < 0)
                    continue;
                if(client[j] == new_sd)
                    continue;
                write(client[j], sbuf, BUFLEN);
            }


            FD_SET (new_sd, &allset);     // add new descriptor to set


            if (new_sd > maxfd)
                maxfd = new_sd;    // for select

            if (i > maxi)
                maxi = i;    // new max index in client[] array

            if (--nready <= 0)
                continue;    // no more readable descriptors
        }

        for (i = 0; i <= maxi; i++)    // check all clients for data
        {
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &rset))
            {
                bp = buf;
                bytes_to_read = BUFLEN;

                while ((n = read(sockfd, bp, bytes_to_read)) > 0)
                {
                    bp += n;
                    bytes_to_read -= n;
                }
                /* displays list of current users and their status */
                if(strstr(buf, "/users"))
                {
                    for(k = 0; k < MAX_CLIENTS; k++)
                    {
                        if(client_list[k].socket > -1)
                        {
                            for(j = 0; j <= maxfd; j++)
                            {
                                if(j == i)
                                {
                                    sprintf(buf, "%c", client_list[k].role);
                                    strcat(buf, " ");
                                    strcat(buf, client_list[k].username);
                                    strcat(buf,"\n");
                                    write(client[j], buf, BUFLEN);
                                }
                            }
                        }
                    }
                    sprintf(buf, "%d", numClients);
                    strcat(buf, " users in chat.\n\n");
                    write(client[i], buf, BUFLEN);
                    break;
                }

                if(strstr(buf, "/msg"))
                {
                    strcpy(msgUser, getName(buf));
                    trim(msgUser);
                    for(k = 0; k < MAX_CLIENTS; k++)
                    {
                        if((client_list[k].socket > -1) && !strcmp(client_list[k].username, msgUser))
                        {
                            for(j = 0; j <= maxfd; j++)
                            {
                                if(j == client_list[k].socket)
                                    write(client[j], buf, BUFLEN);
                            }
                        }
                    }
                    break;
                }

                if(strstr(buf, "/kick"))
                {
                    if(check_permission(i))
                    {
                        buf[strlen(buf) - 1] = 0;
                        strcpy(msgUser, getName(buf));
                        trim(msgUser);
                        for(k = 0; k < MAX_CLIENTS; k++)
                        {
                            if((client_list[k].socket > -1) && !strcmp(client_list[k].username, msgUser))
                            {
                                for(j = 0; j <= maxfd; j++)
                                {
                                    if(j == client_list[k].socket)
                                    {
                                        write(client[j], "you have been kicked\n", BUFLEN);
                                        removeClient(j);
                                        client[j] = -1;
                                    }
                                }
                            }
                        }
                        break;
                    }else
                        write(client[i], "you don't have permission\n", BUFLEN);
                }
                /* promote a user to admin or remove admin status from user */
                if(strstr(buf, "/promote") || strstr(buf, "/demote"))
                {
                    if(check_permission(i))
                    {
                        if(strstr(buf, "/promote"))
                            promotion = 1;
                        else
                            promotion = 0;

                        buf[strlen(buf) - 1] = 0;
                        strcpy(msgUser, getName(buf));
                        trim(msgUser);
                        for(k = 0; k < MAX_CLIENTS; k++)
                        {
                            if((client_list[k].socket > -1) && !strcmp(client_list[k].username, msgUser))
                            {
                                for(j = 0; j <= maxfd; j++)
                                {
                                    if(j == client_list[k].socket)
                                    {
                                        if(promotion)
                                        {
                                            client_list[j].role = 'a';
                                            write(client[j], "you have been promoted!!\n", BUFLEN);
                                        }else
                                        {
                                            client_list[j].role = 'n';
                                            write(client[j], "you have been demoted\n", BUFLEN);
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }else
                        write(client[i], "you don't have permission\n", BUFLEN);
                }
                /* disconnected signal */
                if(strstr(buf, "has left the chat"))
                {
                    for(j = 0; j <= maxfd; j++)
                    {
                        if(j == i)
                        {
                            removeClient(j);
                            client[i] = -1;
                        }

                        else
                            write(client[j], buf, BUFLEN);
                    }
                    break;
                }

                /* if users enters invalid command */
                if(strstr(buf, "/"))
                {
                    write(client[i], "type /help for a list of commands\n", BUFLEN);
                }else
                {
                    for(j = 0; j <= maxfd; j++)
                    {
                        if(j != i)
                            write(client[j], buf, BUFLEN);
                    }
                }

                if (--nready <= 0)
                    break;        // no more readable descriptors
            }
        }
    }
    close(sockfd);
    exit(0);
}

/* kills process on termination */
void catch_int_server(int signo)
{
    if(signo == SIGINT)
    {
        char sbuf[BUFLEN] = "terminate";
        int j;
        for(j = 0; j < (int)(sizeof(client) / sizeof(client[0])); j++)
        {
            if(client[j] > -1)
                write(client[j], sbuf, BUFLEN);
        }
        kill(getpid(), SIGTERM);
    }
}

