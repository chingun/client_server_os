#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#define BUFSIZE 520
#define LOCALHOST "127.0.0.1"


#define USAGE                                                \
    "usage:\n"                                               \
    "  transferclient [options]\n"                           \
    "options:\n"                                             \
    "  -s                  Server (Default: localhost)\n"    \
    "  -p                  Port (Default: 20502)\n"           \
    "  -o                  Output file (Default cs6200.txt)\n" \
    "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"output", required_argument, NULL, 'o'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 20502;
    char *filename = "cs6200.txt";

    setbuf(stdout, NULL);

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:o:hx", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 's': // server
            hostname = optarg;
            break;
        case 'p': // listen-port
            portno = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        case 'o': // filename
            filename = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    if (NULL == hostname)
    {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == filename)
    {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535))
    {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    /* Socket Code Here */
    if (strcmp(hostname,"localhost")==0){
        hostname = LOCALHOST;
    }
    
    int sock;
    char buffer[BUFSIZE];
    struct addrinfo hints, *serverinfo, *addr;
    int numAddrs, recvMsgSize, fwriteMsgSize;
    char portno_char[6];
    
    sprintf(portno_char, "%d", portno);     // Convert portnum for getaddrinfo.
    memset(&hints, 0, sizeof hints);        // HINTS: narrows down what type of address the client is looking for
    
    hints.ai_family = AF_INET;              // This field specifies the desired address family for the
                                            // returned addresses.  Valid values for this field include
                                            // AF_INET and AF_INET6.  The value AF_UNSPEC indicates that
                                            // getaddrinfo() should return socket addresses for any
                                            // address family (either IPv4 or IPv6, for example) that
                                            // can be used with node and service.
    
    hints.ai_socktype = SOCK_STREAM;        // This field specifies the preferred socket type, for exam‐
                                            // ple SOCK_STREAM or SOCK_DGRAM.  Specifying 0 in this
                                            // field indicates that socket addresses of any type can behostname
                                            // returned by getaddrinfo().

    if ((numAddrs = getaddrinfo(hostname, portno_char, &hints, &serverinfo)) != 0) { // Finds all the addresses to connect to
        fprintf(stderr, "The client failed to get address info with issue: %i\n", numAddrs);
        exit(1);
    }
    
    // Create file in C
    FILE* file_on_client;
    file_on_client = fopen(filename, "a+");
    if(file_on_client == NULL){
        fprintf(stderr, "FAIL: file could not be opened.");
        exit(1);
    }
    
    
    // loop through and connect to the first
    for(addr = serverinfo; addr != NULL; addr = addr->ai_next) {
        // #1 here created socket successfully
        if ((sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1){
            fprintf(stderr, "Failed to generate correct socket, moving to next socket found");
            continue; // immediately starts next loop
        }
        // #2 Connected To Socket successfully
        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == -1){
            close(sock);
            fprintf(stderr, "Failed to connect to socket moving to next socket");
            continue; // immediately starts next loop
        }
        break;
    }
    freeaddrinfo(serverinfo); //clearing memory to stop memory leaks
    if(addr == NULL){
        fprintf(stderr, "Failed to connect to server.");
        exit(1);
    }
    
    // #3 Receive file from Server
    if (recvMsgSize = recv(sock, buffer, MSG_BUFFER - 1, 0) < 0){
        fprintf(stderr, "send() recv failed receive message size was %i\n", recvMsgSize);
        exit(1);
    }
    while (recvMsgSize > 0) {
        if((fwriteMsgSize = fwrite(buffer, 1, recvMsgSize, clientfile)) < 0){
            fprintf(stderr, "fwrite() failed receive message size was %i\n", fwrite);
            exit(1);
        }
        
        if ((recvMsgSize = recv(mySocket, messageBuffer, BUFSIZE-1, 0)) < 0){
            fprintf(stderr, "recv() failed to receive message it recv returned %i\n");
            exit(1);
        }
    }

}
