CC=gcc
FLAGS=-W -Wall
LFLAGS=
OUTPUTFILENAME=lmsn

main: main.o client.o server.o server_util.o
	$(CC) $(LFLAGS) main.o client.o server.o server_util.o -o $(OUTPUTFILENAME)

clean:
	rm -f *.o core.* $(OUTPUTFILENAME)

main.o: main.c
	$(CC) $(FLAGS) -c main.c

client.o: client.c
	$(CC) $(FLAGS) -c client.c

server.o: server.c
	$(CC) $(FLAGS) -c server.c
	
server_util.o: server_util.c
	$(CC) $(FLAGS) -c server_util.c
