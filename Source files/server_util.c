#include "main.h"

extern CList client_list[MAX_CLIENTS];
extern int numClients;

// Prints the error stored in errno and aborts the program.
void SystemFatal(const char* message)
{
    perror (message);
    exit (EXIT_FAILURE);
}

/* create a listening socket for server */
void listen_socket(int *listen_sd, struct sockaddr_in *server, int port)
{
    int arg;
    // Create a stream socket
    if ((*listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        SystemFatal("Cannot Create Socket!");

    // set SO_REUSEADDR so port can be resused imemediately after exit, i.e., after CTRL-c
    arg = 1;
    if (setsockopt (*listen_sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
        SystemFatal("setsockopt");

    // Bind an address to the socket
    memset(server, 0, sizeof(struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client
}

/* adds a client to list by setting role, username and socket
-- returns 0 on success
*/
int addClient(int pos, int sock, char r, char *name, char *addr)
{
    client_list[pos].socket = sock;
    client_list[pos].role = r;
    strcpy(client_list[pos].username, name);
    strcpy(client_list[pos].ip, addr);

    numClients++;

    print_clients();

    return 0;
}

/* remove a client from list by resetting socket and role */
int removeClient(int sock)
{
    int k;

    for(k = 0; k < MAX_CLIENTS; k++)
    {
        /* checks for specific client */
        if(client_list[k].socket == sock)
        {
            printf("%s has disconnected with remote address of %s\n", client_list[k].username, client_list[k].ip);
            /* reset client */
            memset(client_list[k].username, 0, sizeof(client_list[k].username));
            memset(client_list[k].ip, 0, sizeof(client_list[k].ip));

            client_list[k].socket = -1;
            client_list[k].role = 'n';
        }
    }
    numClients--;

    print_clients();

    return 0;
}

/* prints list of clients on server side */
void print_clients()
{
    int k;

    printf("====================================\n");
    for(k = 0; k < MAX_CLIENTS; k++)
    {
        if(client_list[k].socket > -1)
        {
            printf("- %s\n", client_list[k].username);
        }
    }
    printf("%d clients\n", numClients);
    printf("====================================\n\n");
}

/* extracts the name from the receive buffer
-- returns name extracted from buffer
*/
char *getName(char *str)
{
    char name[30];
    char *end;
    char *copy = strdup(str);
    char *delim = " ";

    strcpy(str,"*");
    strcat(str, strtok(copy, delim));
    strcat(str, " ");
    strtok(NULL, delim);
    strcpy(name, strtok(NULL, delim));
    if((end = strtok(NULL, "\n")) != NULL)
    {
        strcat(str, end);
        strcat(str, "\n");
    }

    return name;
}

/* trims whitespace from a string
-- returns string with trimmed whitespace
*/
char *trim(char *str)
{
  char *end;

  /* trim leading space */
  while(isspace(*str))
    str++;

  if(*str == 0)
    return str;

  /* trim trailing space */
  end = str + strlen(str) - 1;

  while(end > str && isspace(*end))
    end--;

  return str;
}

/* checks permission of user
-- returns 1 if admin otherwise 0
*/
int check_permission(int c)
{
    int i;

    for(i = 0; i < MAX_CLIENTS; i++)
        if(client_list[i].socket == c && client_list[i].role == 'a')
            return 1;
    return 0;
}
