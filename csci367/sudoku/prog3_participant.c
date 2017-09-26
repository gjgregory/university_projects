// prog3_participant.c
// Garrett Gregory
// CSCI-367

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* code is base on demo_client.c (provided)
*/

/*------------------------------------------------------------------------
* Program: Participant
*
* Purpose: allocate a socket, connect to a server, and enter player name/moves
*
* Syntax: participant [ host [port] ]
*
* host - name of a computer on which server is executing
* port - protocol port number server is using
*
*
*------------------------------------------------------------------------
*/
int main( int argc, char **argv) {
   struct hostent *ptrh; /* pointer to a host table entry */
   struct protoent *ptrp; /* pointer to a protocol table entry */
   struct sockaddr_in sad; /* structure to hold an IP address */
   int sd; /* socket descriptor */
   int port; /* protocol port number */
   char *host; /* pointer to host name */
   int n; /* number of characters read */
   char playerMove[3];

   memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
   sad.sin_family = AF_INET; /* set family to Internet */

   if( argc != 3 ) {
      fprintf(stderr,"Error: Wrong number of arguments\n");
      fprintf(stderr,"usage:\n");
      fprintf(stderr,"./client server_address server_port\n");
      exit(EXIT_FAILURE);
   }

   port = atoi(argv[2]); /* convert to binary */
   if (port > 0) /* test for legal value */
   sad.sin_port = htons((u_short)port);
   else {
      fprintf(stderr,"Error: bad port number %s\n",argv[2]);
      exit(EXIT_FAILURE);
   }

   host = argv[1]; /* if host argument specified */

   /* Convert host name to equivalent IP address and copy to sad. */
   ptrh = gethostbyname(host);
   if ( ptrh == NULL ) {
      fprintf(stderr,"Error: Invalid host: %s\n", host);
      exit(EXIT_FAILURE);
   }

   memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

   /* Map TCP transport protocol name to protocol number. */
   if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
      fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
      exit(EXIT_FAILURE);
   }

   /* Create a socket. */
   sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
   if (sd < 0) {
      fprintf(stderr, "Error: Socket creation failed\n");
      exit(EXIT_FAILURE);
   }

   /* Connect the socket to the specified server. */
   if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
      fprintf(stderr,"connect failed\n");
      exit(EXIT_FAILURE);
   }

   // get the player's name and send it (repeat until server confirms validity)
   char playerName[1000] = "";
   char nameConf;
   while (nameConf != 'Y'){
      printf("Enter your name (max 30 characters): ");
      scanf("%[^\n]%*c", playerName);
      n = send(sd, &playerName, sizeof(playerName),0);
      n = recv(sd, &nameConf, sizeof(nameConf), 0);
      if (n <= 0){
         printf("There's not enough room in the game.\n");
         exit(EXIT_FAILURE);
      }
      printf("confirmation is %c\n", nameConf);
   }

   /* Playing the game... */

   while (1) {
      //get move from player
      printf("Enter your move: ");
      scanf("%[^\n]%*c", playerMove);
      n = send(sd, &playerMove, sizeof(playerMove),0);
   }
}
