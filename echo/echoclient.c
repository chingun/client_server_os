#include <string.h>         //
#include <unistd.h>         //
#include <errno.h>          //

#include <stdlib.h>         // Standard Library
#include <stdio.h>          // Standard Input Output

#include <netdb.h>          //
#include <getopt.h>         //

#include <sys/socket.h>     // Definitions of socket functions like send recv
#include <sys/types.h>      // Definitions of socket functions like send recv

#include <netinet/in.h>     // Contains structures to store address information
                            // Used to connect to other sockets

/* A buffer large enough to contain the longest allowed string */
#define BUFSIZE 520

/* Personal variable names */
#define LOCALHOST "127.0.0.1"
#define MSG_BUFFER 16               //neither the message to the server nor the response will be longer than 15 bytes
/* End */

#define USAGE                                                                       \
    "usage:\n"                                                                      \
    "  echoclient [options]\n"                                                      \
    "options:\n"                                                                    \
    "  -s                  Server (Default: localhost)\n"                           \
    "  -p                  Port (Default: 20502ls)\n"                                  \
    "  -m                  Message to send to server (Default: \"Hello world.\")\n" \
    "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"message", required_argument, NULL, 'm'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 20502;
    char *message = "Hello World!!";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:hx", gLongOptions, NULL)) != -1)
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
        case 'm': // message
            message = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535))
    {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (NULL == message)
    {
        fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == hostname)
    {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */
    int sock;                        /* Socket descriptor */
    char echoBuffer[BUFSIZE];     /* Buffer for echo string */
    int numAddrs;
    int sendMsgSize;
    int recvMsgSize;
    // int yes = 1;
    char portno_char[6];
    // unsigned int echoStringLen;      /* Length of string to echo */
    // int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
                                        // and total bytes read */

    // #1 Create Socket: Has to be more elegant with getaddrinfo to account for Bonnie cases
    struct addrinfo hints, *serverinfo, *addr;
    memset(&hints, 0, sizeof hints);    // HINTS: narrows down what type of address the client is looking for
    hints.ai_family = AF_INET;          // This field specifies the desired address family for the
                                        // returned addresses.  Valid values for this field include
                                        // AF_INET and AF_INET6.  The value AF_UNSPEC indicates that
                                        // getaddrinfo() should return socket addresses for any
                                        // address family (either IPv4 or IPv6, for example) that
                                        // can be used with node and service.
    hints.ai_socktype = SOCK_STREAM;    // This field specifies the preferred socket type, for exam‐
                                        // ple SOCK_STREAM or SOCK_DGRAM.  Specifying 0 in this
                                        // field indicates that socket addresses of any type can behostname
                                        // returned by getaddrinfo().

    sprintf(portno_char, "%d", portno);     // Convert portnum for getaddrinfo.

    if(strcmp(hostname, "localhost")==0){
        hostname = LOCALHOST;
    }

    if ((numAddrs = getaddrinfo(hostname, portno_char, &hints, &serverinfo)) != 0) {
        fprintf(stderr, "The client failed to get address info with issue: %i\n", numAddrs);
        exit(1);
    }

    // loop through and connect to the first
    for(addr = serverinfo; addr != NULL; addr = addr->ai_next) {
        // #1 here created socket successfully  
        if ((sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1){
            fprintf(stderr, "Failed to generate correct socket, moving to next socket found");
            continue;
        }
        // #2 Connect To Socket
        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == -1){
            close(sock);
            fprintf(stderr, "Failed to connect to socket moving to next socket");
            continue;
        }
        break;
    }
    // Might not be useful in client: setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    freeaddrinfo(serverinfo); // otherwise ==14651==ERROR: LeakSanitizer: detected memory leaks

    // #3 Send to Server
    sendMsgSize = send(sock, message, MSG_BUFFER - 1, 0);
    if (sendMsgSize != MSG_BUFFER - 1){
        fprintf(stderr, "send() sent a different number of bytes than expected");
        exit(1);
    }

    // #4 Recieve from Server
    /* Receive up to the buffer size (minus 1 to leave space for
    a null terminator) bytes from the sender */
    recvMsgSize = recv(sock, echoBuffer, MSG_BUFFER - 1, 0);
    if (recvMsgSize < 0){
        fprintf(stderr, "recv() failed or connection closed prematurely");
        exit(1);
    }

    echoBuffer[recvMsgSize] = '\0';
    // totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
    // echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
    // #5 Print the message
    fprintf(stdout, "%s", echoBuffer);      /* Print the echo buffer */
    
    return 0;
}


