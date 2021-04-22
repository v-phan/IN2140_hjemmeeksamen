//Sekvensnummer pakke starter på 1 ved client's connection request.

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

unsigned char *createPacket( unsigned char *flag,
    unsigned char *pktseq,
    unsigned char *ackseq,
    int senderID,
    int recvID,
    int metadata,
    unsigned char *payload){
        unsigned char *packet = malloc(20+1);
        packet[0] = flag;
        packet[1] = pktseq;
        packet[2] = ackseq;
        packet[3] = unassigned;
        packet[4] = (unsigned char)senderID;
        packet[5] = (unsigned char)recvID;
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

//Metode for å akseptere connection request fra en client
int rdp_accept(int senderID, int recvID){
    unsigned char *packet = createPacket(0x10,2,1,senderID, recvID,0b00001000,0x00);
    sendMessage(packet);
    rdp_listen();
    
}

void rdp_refuse(int senderID, int recvID){
    unsigned char *packet = createPacket(0x20,2,1,senderID, recvID,0b00001000,0x00);
    sendMessage(packet);
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
        printf("Connection request from  client %i: Sending accept connection - packet\n", recvID);
        rdp_accept(ID,recvID);
        int connected;
        rdp_listen();
    }
    else if(flag == 0x02){
        printf("This is a connect termination\n");

    }
    else if(flag == 0x04){
        printf("This packet contains data\n");

    }
    else if(flag == 0x08){
        if(pktseq == 3){
            printf("Success, we are connected, BABY! Server %i connected to client %i\n", ID, recvID);
        }
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

//Søker etter nye tilkoblinger
void rdp_listen(){
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
    close(fd);

    check_flags(message[0], message);

}

int rdp_accept_ack(int senderID){
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
    close(fd);
    message[rc] = 0;
    printf("Running check of accept ack %i\n", message[4]);
    if(message[0] == 0x08 && senderID == message[4]){
        return 1;
    }
    else{
        check_flags(message[0], message);   
        return 0;
    }

}

int main (int argc, char *argv[]){
    //oppgavekrav
    // check_arguments(argc, argv);
    // set_loss_probability(prob);

    // rdp_listen();
    while(1){
        rdp_listen();
    }


    return EXIT_SUCCESS;

   
}