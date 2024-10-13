#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT "2520"
#define PLANS_FILE "deathstarplans.dat"

typedef struct {
    char * data;
    int length;
} buffer;

extern int errno;

// ece-nebula11.eng.uwaterloo.ca

/* This function loads the file of the Death Star
   plans so that they can be transmitted to the
   awaiting Rebel Fleet. It takes no arguments, but
   returns a buffer structure with the data. It is the
   responsibility of the caller to deallocate the 
   data element inside that structure.
   */ 
buffer load_plans( );

int main( int argc, char** argv ) {

    if ( argc != 2 ) {
        printf( "Usage: %s IP-Address\n", argv[0] );
        return -1;
    }
    printf("Planning to connect to %s.\n", argv[1]);

    buffer buf = load_plans();

    struct addrinfo hints;
    struct addrinfo *res;
    int sockfd;

    memset(&hints, 0, sizeof( hints ));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int result = getaddrinfo(argv[1], PORT, &hints, &res);
    if(result != 0){
        perror("getaddrinfo");
        return 1;
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if(status != 0){
        perror("connect");
        return 1;
    }
    
    int send_result = sendall(sockfd, &buf);
    if(send_result != 0){
        perror("Send all");
        return 1;
    }

    char result_from_serevr[65] = {0};
    int recv_result = recv(sockfd, result_from_serevr, sizeof(result_from_serevr), 0);
    if (recv_result != 0){
        perror("recv");
        return 1;
    }

    printf("%s", result_from_serevr);

    free(buf.data);
    freeaddrinfo(res);
    close(sockfd);
    return 0;
}

buffer load_plans( ) {
    struct stat st;
    stat( PLANS_FILE, &st );
    ssize_t filesize = st.st_size;
    char* plansdata = malloc( filesize );
    int fd = open( PLANS_FILE, O_RDONLY );
    memset( plansdata, 0, filesize );
    read( fd, plansdata, filesize );
    close( fd );

    buffer buf;
    buf.data = plansdata;
    buf.length = filesize;

    //free(plansdata);
    // need to deallocate!!!!!
    return buf;
}


int sendall(int sockfd, buffer * buf) {

    int total = 0;         // Bytes sent so far
    int bytesleft = buf->length;   // Bytes left to send
    int n;

    while (total < buf->length) {
        n = send(sockfd, buf->data + total, bytesleft, 0);
        if (n == -1) {
            perror("While loop send");
            break;;
        }
        total += n;
        bytesleft -= n;
    }

    buf->length = total;  // Update the actual number of bytes sent
    return n == -1 ? -1 : 0;  // Return -1 on failure, 0 on success
}