
CFLAGS = -std=gnu11 -g -Wall -Wextra
BIN = client server

all: $(BIN)

server: server.o send_packet.o send_packet.h 
	gcc $(CFLAGS) server.o send_packet.o -o server 

client: client.o send_packet.o send_packet.h 
	gcc $(CFLAGS) client.o send_packet.o -o client 

client.o: client.c
	gcc $(CFLAGS) -c client.c

server.o: server.c
	gcc $(CFLAGS) -c server.c

sendpacket.o: send_packet.c
	gcc $(CFLAGS) -c send_packet.c

val: 
	valgrind --leak-check=full ./client 


clean:
	rm -f $(BIN) *.o