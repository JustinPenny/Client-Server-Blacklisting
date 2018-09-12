all: client pserver

client: client.c
	gcc -std=c99 -o client client.c
    
server: pserver.c
	gcc -std=c99 -o pserver pserver.c
