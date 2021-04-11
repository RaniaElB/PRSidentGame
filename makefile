all : 
	make server 
	make client

server : server.c
	gcc server.c -o server.o -lpthread
	
client : client.c
	gcc client.c -o client.o -lpthread -std=gnu99
	
clean:
	rm -f *.o
