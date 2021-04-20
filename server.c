#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "send_packet.h"

#define myPORT 2021
#define yourPORT 2022
#define IP "127.0.0.1"
#define BUFSIZE 255


unsigned char *unassigned = 0x00;

float prob;
int port, files;
int ID = 77;
char *filename;

void check_error(int res, char *msg){
    if (res == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

void rdp_accept(){

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

//Behandler argumenter fra kommandolinje
void check_arguments(int argc, char *argv[]){
    
    if(argc != 5){
        printf("feil antall argumenter");
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);
    filename = argv[2]; 
    files = atoi(argv[3]);
    char *s = argv[4];
    int i = 0;
    while(s[i] != '\0'){
        if(s[i] == ','){
            s[i] = '.';
        }
        i++;
    }
    sscanf(s,"%f",&prob);
}


void sendMessage(unsigned char *packet){
    int fd, rc, wc;

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
                sizeof(struct sockaddr_in));

    check_error(wc, "sendto");
    free(packet);
    close(fd);
}

void check_flags(unsigned char *flag, unsigned char message[]){
    unsigned char *pktseq, *ackseq, *payload, *metadata;
    int senderID, recvID;
    pktseq = message[1];
    ackseq = message[2];
    recvID = message[4];
    metadata = message[6];
    payload = message[7];

    if(flag == 0x01){
        printf("This is a connect request: Sending accept connection - packet\n");
        unsigned char *packet = createPacket(0x10,0x00,0x00,ID, recvID,0b00001000,0x00);
        // unsigned char packet[8];
        // metadata = 0b00001000;
        // packet[0] = 0x10;
        // packet[1] = "";
        // packet[2] = "";
        // packet[3] = unassigned;
        // packet[4] = senderID;
        // packet[5] = recvID;
        // packet[6] = metadata;
        // packet[7] = "";
        sendMessage(packet);
        printf("Success, we are connected, BABY! Server %i connected to client %i\n", ID, recvID);
    }
    else if(flag == 0x02){
        printf("This is a connect termination\n");

    }
    else if(flag == 0x04){
        printf("This packet contains data\n");

    }
    else if(flag == 0x08){
        printf("This packet is an ACK\n");
    }
    else if(flag == 0x10){
        printf("This packet accepts a connect request\n");
    }
    else if(flag == 0x20){
        printf("This packet refuses a connect request\n");
    }
    else{
        printf("Packet not recognized: Vi mottok\n");
    }

}

void listenTo(){
     int fd, rc;
    
    struct sockaddr_in my_addr, src_addr;
    unsigned char message[BUFSIZE];

    fd = socket(AF_INET, SOCK_DGRAM,0);

    check_error(fd,"socket");

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    check_error(rc, "bind");
    rc = recv(fd, message, BUFSIZE-1,0 );
    check_error(rc, "recv");
    message[rc] = 0;

    check_flags(message[0], message);

    close(fd);
}

int main (int argc, char *argv[]){
    //oppgavekrav
    // check_arguments(argc, argv);
    // set_loss_probability(prob);

    // listenTo();
    while(1){
        listenTo();
    }


    return EXIT_SUCCESS;

   
}