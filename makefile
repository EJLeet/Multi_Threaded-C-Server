all: client server

client: client.c
	gcc -o client client.c -lpthread -lm

server: server.c
	gcc -o server server.c -lpthread -lm