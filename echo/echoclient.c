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
    int network_socket; 
    network_socket = socket(AF_INET, SOCK_STREAM, 0); //This is how a TCP network socket is created

    // Specifiy an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = portno; //
    server_address.sin_addr.s_addr = INADDR_ANY; //Data of the address
    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    
    // Check for error with the connection
    if(connection_status == -1){
        printf("There was an error making a connection to the remote socket\n\n");
    }

    // Receive data from the server
    char server_response[BUFSIZE];
    send(network_socket, message, 16, 0);
    // printf("Message sent: %s\n", message);
    recv(network_socket, &server_response, 16, 0);
    // print out the server's response
    printf("%s", server_response);
    // Finally close the socket
    close(network_socket);
    return 0;
}


