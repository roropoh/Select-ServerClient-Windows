#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/mman.h>

#define LISTENQ 5
#define MAX_CLIENTS 10
#define BUFLEN  512

#define SYN 0x16
#define EOT 0x04

typedef struct str_client_list
{
    int socket;
    char role;
    char username[32];
    char ip[32];
} CList;

typedef struct file_info
{
    FILE *fp;
    int log;
}FInfo;

//Server
void startServer(int port);

void SystemFatal(const char* );
void listen_socket(int *listen_sd, struct sockaddr_in *server, int port);
int addClient(int pos, int sock, char r, char *name, char *addr);
int removeClient(int sock);
char *getName(char *str);
char *trim(char *str);
void print_clients();
void catch_int(int signo);
void catch_int_server(int signo);
int check_permission(int c);

//Client
void startClient(char* username, char* ip, int port);

//Main
void exit_message();

#endif
