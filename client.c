#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include "send_packet.h"

#define myPORT 2022
#define yourPORT 2021
#define IP "127.0.0.1"
#define BUFSIZE 255


float prob;
int ID;
int port;
char *ip;
unsigned char *unassigned = 0x00;

void check_error(int res, char *msg){
    if (res == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

//Behandler argumenter fra kommandolinje
void check_arguments(int argc, char *argv[]){
    
    if(argc != 4){
        printf("feil antall argumenters");
        exit(EXIT_FAILURE);
    }
    ip = argv[1]; 
    port = atoi(argv[2]);
    char *s = argv[3];
    int i = 0;
    while(s[i] != '\0'){
        if(s[i] == ','){
            s[i] = '.';
        }
        i++;
    }
    sscanf(s,"%f",&prob);
}

unsigned char *createPacket( unsigned char *flag,
    unsigned char *ackseq,
    unsigned char *pktseq,
    int senderID,
    int recvID,
    int metadata,
    unsigned char *payload){
        unsigned char *packet = malloc(20+1);
        packet[0] = flag;
        packet[1] = pktseq;
        packet[2] = ackseq;
        packet[3] = unassigned;
        packet[4] = senderID;
        packet[5] = recvID;
        packet[6] = metadata;
        packet[7] = payload;
        packet[8] = '\0';
        return packet;
}



void sendConnectReq(){

    int fd, wc;
    unsigned char *packet = createPacket(0x01,0x00,0x00,ID, 0,0b00001000,0x00);
    // unsigned char *pktseq, *ackseq, *payload;
    // int *senderID, *recvID;
    // pktseq = "";
    // ackseq = "";
    // payload = "";
    
    // recvID = 0;
    // senderID = ID;

    // unsigned char *flag = 0x01;
    // unsigned char *metadata = 0b00001000;
    // char packet[8];
    // packet[0] = flag;
    // packet[1] = pktseq;
    // packet[2] = ackseq;
    // packet[3] = unassigned;
    // packet[4] = senderID;
    // packet[5] = recvID;
    // packet[6] = metadata;
    // packet[7] = payload;



    struct sockaddr_in dest_addr;
    struct in_addr ip_addr;
    wc = inet_pton(AF_INET,IP,&ip_addr.s_addr);

    fd = socket(AF_INET, SOCK_DGRAM,0);

    check_error(fd,"socket");
    if(!wc){
        fprintf(stderr, "Invalid IP adress: %s\n", IP);
    }
    
    dest_addr.sin_family = AF_INET;
    //htons gjør om til network byte order
    dest_addr.sin_port = htons(yourPORT);
    dest_addr.sin_addr = ip_addr;

    wc = sendto(fd,
                packet, 
                sizeof(packet), 
                0, 
                (struct sockaddr*)&dest_addr, 
                sizeof(struct sockaddr_in) );

    check_error(wc, "sendto");
    close(fd);
    free(packet);
    listenTo();

}

void sendMessage(){
    int fd,  wc;
    
    unsigned char *msg = 0x01;
    unsigned char *metadata = 0b00001000;
    char buf[BUFSIZE];
    buf[0] = msg;
    buf[1] = metadata;

    struct sockaddr_in dest_addr;
    struct in_addr ip_addr;
    wc = inet_pton(AF_INET,IP,&ip_addr.s_addr);

    fd = socket(AF_INET, SOCK_DGRAM,0);

    check_error(fd,"socket");
    if(!wc){
        fprintf(stderr, "Invalid IP adress: %s\n", IP);
    }
    
    dest_addr.sin_family = AF_INET;
    //htons gjør om til network byte order
    dest_addr.sin_port = htons(yourPORT);
    dest_addr.sin_addr = ip_addr;

    wc = sendto(fd,
                buf, 
                sizeof(buf), 
                0, 
                (struct sockaddr*)&dest_addr, 
                sizeof(struct sockaddr_in) );

    check_error(wc, "sendto");
    close(fd);
    listenTo();

}

void listenTo(){
     int fd, rc;

    
    struct sockaddr_in my_addr, src_addr;
    unsigned char packet[BUFSIZE];

    fd = socket(AF_INET, SOCK_DGRAM,0);

    check_error(fd,"socket");

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    check_error(rc, "bind");
    rc = recv(fd, packet, BUFSIZE-1,0 );
    check_error(rc, "recv");
    packet[rc] = 0;

    check_flags(packet[0], packet);
    close(fd);
    return EXIT_SUCCESS;
    
}

void check_flags(unsigned char *flags, unsigned char message[]){
    unsigned char *pktseq, *ackseq, *payload, *metadata;
    int serverID, clientID;
    pktseq = message[1];
    ackseq = message[2];
    serverID = message[4];
    clientID = message[5];

    // senderID = ID;
    metadata = message[6];
    payload = message[7];
    

    if(flags == 0x01){
        printf("This is a connect request: Sending accept connection - packet\n ");
    }
    else if(flags == 0x02){
        printf("This is a connect termination \n");
        return EXIT_SUCCESS;
    }
    else if(flags == 0x04){
        printf("This packet contains data\n");
    }
    else if(flags == 0x08){
        printf("This packet is an ACK\n");
    }
    else if(flags == 0x10){
        connected = 1;
        printf("We are connected baby! Client %i is connected to server %i\n", clientID, serverID );
    }
    else if(flags == 0x20){
        printf("This packet refuses a connect request \n");
    }
    else{
        printf("Packet not recognized\n");
    }

}

int main (int argc, char *argv[]){
    //oppgavekrav
    // check_arguments(argc, argv);
    // set_loss_probability(prob);
    // unsigned char *flags, *pktseq, *ackseq, unassigned;
    // int senderID, recvID, metadata, payload;
    srand(time(NULL));
    ID = rand() % 100;
    
    sendConnectReq();
    

    
}