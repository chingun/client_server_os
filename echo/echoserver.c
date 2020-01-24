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

/* Personal variable names */
#define LOCALHOST "127.0.0.1"
#define MSG_BUFFER 16               //neither the message to the server nor the response will be longer than 15 bytes
/* End */

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoserver [options]\n"                                                    \
"options:\n"                                                                  \
"  -p                  Port (Default: 20502)\n"                                \
"  -m                  Maximum pending connections (default: 1)\n"            \
"  -h                  Show this help message\n"                              \

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
  {"port",          required_argument,      NULL,           'p'},
  {"maxnpending",   required_argument,      NULL,           'm'},
  {"help",          no_argument,            NULL,           'h'},
  {NULL,            0,                      NULL,             0}
};

void DieWithError(char *errorMessage);  /* Error handling function */
void HandleTCPClient(int clntSocket);   /* TCP client handling function */

int main(int argc, char **argv) {
  int option_char;
  int portno = 20502; /* port to listen on */
  int maxnpending = 1;
  
  // Parse and set command line arguments
  while ((option_char = getopt_long(argc, argv, "p:m:hx", gLongOptions, NULL)) != -1) {
   switch (option_char) {
      case 'p': // listen-port
        portno = atoi(optarg);
        break;                                        
      default:
        fprintf(stderr, "%s ", USAGE);
        exit(1);
      case 'm': // server
        maxnpending = atoi(optarg);
        break; 
      case 'h': // help
        fprintf(stdout, "%s ", USAGE);
        exit(0);
        break;
    }
  }

  setbuf(stdout, NULL); // disable buffering

  if ((portno < 1025) || (portno > 65535)) {
      fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
      exit(1);
  }
  if (maxnpending < 1) {
      fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__, maxnpending);
      exit(1);
  }

  /* Socket Code Here */
  int servSock;
  int clntSock;
  // struct sockaddr_in echoServAddr; /* Local address */
  struct sockaddr_in echoClntAddr; /* Client address */         
  // char client_msg[16];
  int numAddrs;
  unsigned int clntLen;            /* Length of client address data structure */
  int yes = 1;
  char* hostname = LOCALHOST;
  char portno_char[6];
  sprintf(portno_char, "%d", portno);     // Convert portnum for getaddrinfo.
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


  // /* #1 Create socket for incoming connections */
  if ((numAddrs = getaddrinfo(hostname, portno_char, &hints, &serverinfo)) != 0) {
      fprintf(stderr, "The server failed to get address info with issue: %i\n", numAddrs);
      exit(1);
  }

  // Loop through and connect to the first
  for(addr = serverinfo; addr != NULL; addr = addr->ai_next) {
    // #1 here created socket successfully  
    if ((servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1){
        fprintf(stderr, "Failed to generate correct socket, moving to next socket found");
        continue;
    }
    // Fixes issues with Bonnie
    if(setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      fprintf(stderr, "Cannot set server socket to be reuseable");
      exit(1);
    }

    // #2 Binded To Socket
    if (bind(servSock, addr->ai_addr, addr->ai_addrlen) == -1){
        close(servSock);
        fprintf(stderr, "Failed to connect to socket moving to next socket");
        continue;
    }
    break;
  }
  freeaddrinfo(serverinfo); // otherwise ==14651==ERROR: LeakSanitizer: detected memory leaks

  if (listen(servSock, 5) < 0)
    printf("listen() failed");  // There will only be one client so this count 5 does not really matter in this test

  // if ((servSock = socketaddrF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  //   printf("socket() failed");    
  // setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  // /* Construct local address structure */
  // memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
  // echoServAddr.sin_family = AF_INET;                /* Internet address family */
  // echoServAddr.sin_addr.s_addr = INADDR_ANY; /* Any incoming interface */
  // echoServAddr.sin_port = portno;                   /* Local port */

  // // #2 servers bind with the socket bind properly IP and port
  // /* #2 Bind to the local address */
  // if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
  //   printf("bind() failed");

  // // #3 Listen to how many clients at once?
  // /* Mark the socket so it will listen for incoming connections */
  // if (listen(servSock, 5) < 0)
  //   printf("listen() failed");  // There will only be one client so this count 5 does not really matter in this test


  for(;;) {
    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);

    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
      printf("accept() failed");

    /* clntSock is connected to a client! */
    // printf("Handling client %d\n", inet_ntoa(echoClntAddr.sin_addr));
    HandleTCPClient(clntSock);

    // // # 4 send or receive
    // recv(client_socket, &client_msg, MSG_BUFFER - 1, 0); // recv uses pointer
    // // printf("Here is the received on server: %s\n", client_msg);
    // printf("%s\n", client_msg);
  }
  // # 5 close server this is NEVER REACHED
  close(servSock);
  return 0; 
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[MSG_BUFFER];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, MSG_BUFFER, 0)) < 0)
        printf("recv() failed #1");

    // /* Send received string and receive again until end of transmission */
    // while (recvMsgSize > 0)      /* zero indicates end of transmission */
    // {
    //     /* Echo message back to client */
    if (send(clntSocket, echoBuffer, MSG_BUFFER, 0) < 0)
        printf("send() failed #1");

    close(clntSocket);    /* Close client socket */
}
