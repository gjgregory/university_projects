// prog3_server.c
// Garrett Gregory
// CSCI-367

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>

#define QLEN 6 /* size of request queue */

/*
* Note: This code is base on demo_server.c (provided) and prog2_server.c (Garrett Gregory and Kyle Dieckman)
*/

/*------------------------------------------------------------------------
* Program: server
*
* Purpose: runs a multiplayer game of sudoku with a board that resets every
* round and a scoreboard that never resets. gets names/moves from participants
* and sends announcments to observers
*
* Syntax: server [ observerPort [ participantPort [ maxObservers [ maxParticipants [ roundTime [ pauseTime ] ] ] ] ] ]
*
*
*
*------------------------------------------------------------------------
*/

int checkForNum(char* board, int idx, char num){
   int i, j;
   /* Determines if there is a row conflict */
   for (i = (idx - (idx%9)); i <= (idx + (9 - (idx%9))); i++){
      if (board[i] == num){
         printf("Horizontal conflict.\n");
         return 0;
      }
   }

   /* Determines if there is a column conflic */
   for (i = (idx%9); i < 72 + (idx%9); i += 9){
      if (board[i] == num){
         printf("Vertical conflict.\n");
         return 0;
      }
   }

   /* Determines if there is a conflict in a sub box */
   for (i = (idx - (idx%3)); i < (idx + (3 - (idx%3))); i++){
      for (j = i - 9 * ((i % 27) / 9); j < i + (9 * (3 - (
         i%27) / 9)); j += 9){
            if (board[j] == num){
               printf("Box conflict.\n");
               return 0;
            }
         }
      }
      return 1;
   }

   /* Check if the board is filled, thus the game being over */
   int boardFilled(char* board) {
      int i;
      for (i = 0; i < 81; i++) {
         if (board[i] == '0')
         return 0;
      }
      return 1;
   }

   /* Processes moves, passes, and invalid commands */
   void processMove(char* message, char* board, char* last) {
      int position = -1;
      int convertedNum = message[1] - '0';
      int convertedNum2 = message[2] - '0';

      //check if properly formatted
      if ((convertedNum > 0 && convertedNum <= 9)
      && (convertedNum2 > 0 && convertedNum2 <= 9)) {
         switch (message[0]) {
            case 'A':
               position = 0 + convertedNum - 1;
               break;
            case 'B':
               position = 9 + convertedNum - 1;
               break;
            case 'C':
               position = 18 + convertedNum - 1;
               break;
            case 'D':
               position = 27 + convertedNum - 1;
               break;
            case 'E':
               position = 36 + convertedNum - 1;
               break;
            case 'F':
               position = 45 + convertedNum - 1;
               break;
            case 'G':
               position = 54 + convertedNum - 1;
               break;
            case 'H':
               position = 63 + convertedNum - 1;
               break;
            case 'I':
               position = 72 + convertedNum - 1;
               break;
            default:
               //printf("Invalid move!\n");
               *last = 'X';
               break;
         }
      }
      // check if move can be made
      if (position != -1 && board[position] == '0' && checkForNum(board,position,message[2])) {
                  board[position] = message[2];
         //printf("Move accepted!\n");
         *last = '1';
      } else {
         //printf("Invalid move!\n");
         *last = 'X';
      }
   }

      /*-----------------------------------------------------------------------------*/
      /*-----------------------------------------------------------------------------*/
      /*-----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
   struct protoent *ptrpp; /* pointer to a protocol table entry */
   struct protoent *ptrpo; /* pointer to a protocol table entry */
   struct sockaddr_in sad; /* structure to hold server's address */
   struct sockaddr_in cad; /* structure to hold client's address */
   int sd1, sd2; /* socket descriptors */
   int obs_port; /* observer port number */
   int par_port; /* participant port number */
   int alen; /* length of address */
   int oproundTime = 1; /* boolean value when we set socket option */
   int n;
   char c;

   char gb[81];
   int i, j, k;
   char validMove;
   char playerMove[3];

   if( argc != 7) {
      fprintf(stderr,"Error: Wrong number of arguments\n");
      fprintf(stderr,"usage:\n");
      fprintf(stderr,"./server  obs port, par port, #obs, #par, rlen, plen \n");
      exit(EXIT_FAILURE);
   }

   int maxObs = atoi(argv[3]);
   int maxPar = atoi(argv[4]);
   int roundTime = atoi(argv[5]);
   int delayTime = atoi(argv[6]);

   if (!(roundTime > 0 && delayTime > 0)){
      fprintf(stderr, "Error: Round Time and Delay Time must be greater than 0");
      exit(EXIT_FAILURE);
   }

   // ---------------------------- PARTICIPANT SOCKET -------------------------

   memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
   sad.sin_family = AF_INET; /* set family to Internet */
   sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

   par_port = atoi(argv[2]); /* convert argument to binary */
   if (par_port > 0) { /* test for illegal value */
      sad.sin_port = htons((u_short)par_port);
   } else { /* print error message and exit */
      fprintf(stderr,"Error: Bad port number %s\n",argv[2]);
      exit(EXIT_FAILURE);
   }
   /* Map TCP transport protocol name to protocol number */
   if ( ((long int)(ptrpp = getprotobyname("tcp"))) == 0) {
      fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
      exit(EXIT_FAILURE);
   }

   /* Create a socket */
   sd1 = socket(PF_INET, SOCK_STREAM, ptrpp->p_proto);
   if (sd1 < 0) {
      fprintf(stderr, "Error: Socket creation failed\n");
      exit(EXIT_FAILURE);
   }

   /* Allow reuse of port - avoid "Bind failed" issues */
   if( setsockopt(sd1, SOL_SOCKET, SO_REUSEADDR, &oproundTime, sizeof(oproundTime)) < 0 ) {
      fprintf(stderr, "Error Setting socket option failed\n");
      exit(EXIT_FAILURE);
   }

   /* Bind a local address to the socket */
   if (bind(sd1, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
      fprintf(stderr,"Error: Bind failed\n");
      exit(EXIT_FAILURE);
   }

   /* Specify size of request queue */
   if (listen(sd1, QLEN) < 0) {
      fprintf(stderr,"Error: Listen failed\n");
      exit(EXIT_FAILURE);
   }

   //--------------------------- OBSERVER SOCKET ------------------------------

   memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
   sad.sin_family = AF_INET; /* set family to Internet */
   sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

   obs_port = atoi(argv[1]); /* convert argument to binary */
   if (obs_port > 0) { /* test for illegal value */
      sad.sin_port = htons((u_short)obs_port);
   } else { /* print error message and exit */
      fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
      exit(EXIT_FAILURE);
   }
   /* Map TCP transport protocol name to protocol number */
   if ( ((long int)(ptrpo = getprotobyname("tcp"))) == 0) {
      fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
      exit(EXIT_FAILURE);
   }

   /* Create a socket */
   sd2 = socket(PF_INET, SOCK_STREAM, ptrpo->p_proto);
   if (sd2 < 0) {
      fprintf(stderr, "Error: Socket creation failed\n");
      exit(EXIT_FAILURE);
   }

   /* Allow reuse of port - avoid "Bind failed" issues */
   if( setsockopt(sd2, SOL_SOCKET, SO_REUSEADDR, &oproundTime, sizeof(oproundTime)) < 0 ) {
      fprintf(stderr, "Error Setting socket option failed\n");
      exit(EXIT_FAILURE);
   }

   /* Bind a local address to the socket */
   if (bind(sd2, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
      fprintf(stderr,"Error: Bind failed\n");
      exit(EXIT_FAILURE);
   }

   /* Specify size of request queue */
   if (listen(sd2, QLEN) < 0) {
      fprintf(stderr,"Error: Listen failed\n");
      exit(EXIT_FAILURE);
   }

   int tempSocket;
   int sel;
   int infoSize;
   int parCount = 0; //keeps track of how many participants are connected
   if (maxPar <= 512)
      infoSize = 1024;
   else
      infoSize = maxPar * 2;
   int scores[infoSize]; //array of player scores
   for (i=0; i<infoSize; i++)
      scores[i] = 0;
   char* names[infoSize]; //array of player names
   for (i=0; i<infoSize; i++)
      names[i] = "";
   char tempname[1000];
   char validName = 'Y';
   time_t start; //for keeping track of round/delay times
   //initialize game board values to 0
   for (i=0; i<81; i++)
      gb[i] = '0';
   //initialize socket arrays
   int obs_socs[maxObs], par_socs[infoSize];
   for (i = 0; i < maxObs; i++)
      obs_socs[i] = 0;
   for (i = 0; i < infoSize; i++)
      par_socs[i] = 0;

   printf("Waiting for participants...\n");
   fd_set readSet; //set of socket descriptors
   fd_set writeSet; //set of socket descriptors
   int maxSD = sd1; //largest value socket descriptor
   int lowScore;
   int top5[5];
   int tempScores[infoSize];
   int spotFound;
   int highScore;
   int highIndex;
   int played;
   if (sd2 > sd1)
      maxSD = sd2;
   char gameMessage[500]; //for sending messages to observers
   int isConnected[infoSize]; //for knowning when a socket descriptor can be closed/reopened
   for (i=0; i<infoSize; i++)
      isConnected[i] = 0;
   int gotSD;
   //enter game board values into board message
   j = 0;
   for (i=57; i < 350; i++){
      if (gameMessage[i] == '*'){
         gameMessage[i] = gb[j];
         if (gameMessage[i] == '0')
            gameMessage[i] = ' ';
      }
   }

   //---------------------------------------------------------------------------------------
   //----------------------------------- MAIN SERVER LOOP ----------------------------------
   //---------------------------------------------------------------------------------------
   while (1) {
      start = time(NULL);
      //run round loop until specified time
      while ((int)time(NULL) - (int)start < roundTime){
         alen = sizeof(cad);

         //add socket descriptors to set readSet
         FD_ZERO(&readSet);
         FD_SET(sd1, &readSet);
         FD_SET(sd2, &readSet);
         maxSD = sd2;

         //add valid participant socket descriptors to both sets
         for (i = 0; i < infoSize; i++){
            if (par_socs[i] > 0)
               FD_SET(par_socs[i], &writeSet);
               FD_SET(par_socs[i], &readSet);
            //for select function
            if ((int)par_socs[i] > (int)maxSD)
               maxSD = par_socs[i];
         }

         //add valid observer socket descriptors to both sets
         for (i = 0; i < maxObs; i++){
            if (obs_socs[i] > 0)
               FD_SET(obs_socs[i], &writeSet);
               FD_SET(obs_socs[i], &readSet);
            //for select function
            if (obs_socs[i] > maxSD)
               maxSD = obs_socs[i];
         }

         //check for activity on sockets list
         sel = select(maxSD+1, &readSet, &writeSet, NULL, 0);

         if (sel < 0)
            perror("select()");

         //check if participant disconnected
         for (i=0; i<infoSize; i++) {
            if (isConnected[par_socs[i]] && FD_ISSET(par_socs[i], &readSet)) {
               if (n = recv(par_socs[i], &c, 1, MSG_PEEK) <= 0) {
                  printf("Player %s has left the game.\n", names[i]);
                  isConnected[par_socs[i]] = 0;
                  parCount--;
                  FD_CLR(par_socs[i], &readSet);
                  FD_CLR(par_socs[i], &writeSet);
               }
            }
         }

         //check if observer disconnected
         for (i=0; i<maxObs; i++) {
            if (isConnected[obs_socs[i]] && FD_ISSET(obs_socs[i], &readSet)) {
               if (n = recv(obs_socs[i], &c, 1, 0) <= 0) {
                  printf("Observer disconnected.\n");
                  isConnected[obs_socs[i]] = 0;
                  FD_CLR(obs_socs[i], &readSet);
                  FD_CLR(obs_socs[i], &writeSet);
               }
            }
         }

         //check for participant names sent
         for (i=0; i<infoSize; i++){
            if (isConnected[par_socs[i]] && FD_ISSET(par_socs[i], &readSet) && strcmp(names[i], "") == 0){
               recv(par_socs[i], tempname, sizeof(tempname),0);
               validName = 'Y';
               //check if name is valid, otherwise re-prompt
               if (strlen(tempname) > 0 && strlen(tempname) <= 30){
                  for (j = 0; j < infoSize; j++){
                     if (strcmp(names[j], tempname) == 0){ //name already in use
                        validName = 'N';
                        printf("Invalid name entered.\n");
                        break;
                     }
                  }
                  for (j = 0; j < strlen(tempname); j++){
                     if (!((tempname[j] >= 'a' && tempname[j] <= 'z') ||
                              (tempname[j] >= 'A' && tempname[j] <= 'Z') ||
                              (tempname[j] >= '0' && tempname[j] <= '9') ||
                              tempname[j] == ' ' || tempname[j] == '_')){
                        validName = 'N';
                        break;
                     }
                  }
               }else{
                  validName = 'N';
                  printf("Invalid name entered.\n");
               }
               send(par_socs[i], &validName, sizeof(validName), 0);
               if (validName == 'Y'){
                  names[i] = strdup(tempname);
                  printf("Player has chosen the name, \"%s\"\n", tempname);
               }
            //check for moves entered by participants
            }else if(isConnected[par_socs[i]] && FD_ISSET(par_socs[i], &readSet) && strcmp(names[i], "") != 0){
               //get move from player
               if (n = recv(par_socs[i], playerMove, sizeof(playerMove), 0) > 0){
                  processMove(playerMove, gb, &validMove);
                  if (validMove == '1'){
                     if (scores[i] < 9999) //this is the max value allowed for my server
                        scores[i]++;
                     printf("%s made THIS outrageous move: %s\n", names[i], playerMove);
                     //check for observers connected and send game board message
                     for (j=0; j<maxObs; j++){
                        if (isConnected[obs_socs[j]]){
                           //enter move and formatted game board into board message
                           snprintf(gameMessage, sizeof(gameMessage),
                                       "%s made valid move %s.\n\n"
                                       " | 1 2 3 | 4 5 6 | 7 8 9 |\n"
                                       "-+-------+-------+-------+\n"
                                       "A| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "B| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "C| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "-+-------+-------+-------+\n"
                                       "D| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "E| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "F| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "-+-------+-------+-------+\n"
                                       "G| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "H| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "I| %c %c %c | %c %c %c | %c %c %c |\n"
                                       "-+-------+-------+-------+\n",
                                       names[i], playerMove,
                                       gb[0],gb[1],gb[2],gb[3],gb[4],gb[5],gb[6],gb[7],gb[8],gb[9],
                                       gb[10],gb[11],gb[12],gb[13],gb[14],gb[15],gb[16],gb[17],gb[18],
                                       gb[19],gb[20],gb[21],gb[22],gb[23],gb[24],gb[25],gb[26],gb[27],
                                       gb[28],gb[29],gb[30],gb[31],gb[32],gb[33],gb[34],gb[35],gb[36],
                                       gb[37],gb[38],gb[39],gb[40],gb[41],gb[42],gb[43],gb[44],gb[45],
                                       gb[46],gb[47],gb[48],gb[49],gb[50],gb[51],gb[52],gb[53],gb[54],
                                       gb[55],gb[56],gb[57],gb[58],gb[59],gb[60],gb[61],gb[62],gb[63],
                                       gb[64],gb[65],gb[66],gb[67],gb[68],gb[69],gb[70],gb[71],gb[72],
                                       gb[73],gb[74],gb[75],gb[76],gb[77],gb[78],gb[79],gb[80]);
                           for (k=0; k<500; k++){
                              if (gameMessage[k] == '0')
                                 gameMessage[k] = ' ';
                           }
                           send(obs_socs[j], &gameMessage, strlen(gameMessage), 0);
                        }
                     }
                  }else{
                     if (scores[i] > -9999) //this is the minimum score on my server
                        scores[i]--;
                     printf("%s tried to make an invalid move. What an IDIOT!\n", names[i]);
                     for (j=0; j<maxObs; j++){
                        if (isConnected[obs_socs[j]]){
                           snprintf(gameMessage, sizeof(gameMessage), "%s attempted to make an invalid move.\n", names[i]);
                           send(obs_socs[j], &gameMessage, strlen(gameMessage), 0);
                        }
                     }
                  }
               }
            }
         }

         //check if participant connected
         if (FD_ISSET(sd1, &readSet)){
            //check if another socket needs to be taken over
            gotSD = 0;
            for (i=0; i<infoSize; i++){
               if (isConnected[par_socs[i]] == 0 && par_socs[i] != 0){
                  close(par_socs[i]);
                  par_socs[i] = 0;
                  gotSD = 1;
                  break;
               }
            }
            if (!gotSD){
               for (i=0; i<maxObs; i++){
                  if (isConnected[obs_socs[i]] == 0 && obs_socs[i] != 0) {
                     close(obs_socs[i]);
                     obs_socs[i] = 0;
                     break;
                  }
               }
            }
            if ((tempSocket = accept(sd1, (struct sockaddr *)&cad, &alen)) < 0){
               printf("Error: Accept Failed\n");
               exit(EXIT_FAILURE);
            }

            isConnected[tempSocket] = 1;
            printf("participant socket descriptor = %d, ip addr = %d, port = %d\n", tempSocket,
                     inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

            if (parCount < maxPar){
               //check for an empty spot to reserve for participant
               spotFound = 0;
               for (i = 0; i < infoSize; i++){
                  if (strcmp(names[i], "") == 0){
                     par_socs[i] = tempSocket;
                     parCount++;
                     printf("added socket to list as participant %d\n", i);
                     spotFound = 1;
                     break;
                  }
               }
               //have to kick an old player
               //find low score so they can be kicked
               if (!spotFound){
                  lowScore = 0;
                  for (i = 0; i < infoSize; i++){
                     if (i < lowScore)
                        lowScore = i;
                  }
                  //check for a spot to steal for participant (low score)
                  for (i = 0; i < infoSize; i++){
                     if (par_socs[i] == 0 && scores[i] == lowScore){
                        par_socs[i] = tempSocket;
                        names[i] = "";
                        parCount++;
                        printf("added socket to list as participant %d\n", i);
                        break;
                     }
                  }
               }
            }
            //already full on participants
            else
               close(tempSocket);
         }

         //check if observer connected
         if (FD_ISSET(sd2, &readSet)){
            //check if another socket needs to be taken over
            gotSD = 0;
            for (i=0; i<infoSize; i++){
               if (isConnected[par_socs[i]] == 0 && par_socs[i] != 0){
                  close(par_socs[i]);
                  par_socs[i] = 0;
                  gotSD = 1;
                  break;
               }
            }
            if (!gotSD){
               for (i=0; i<maxObs; i++){
                  if (isConnected[obs_socs[i]] == 0 && obs_socs[i] != 0) {
                     close(obs_socs[i]);
                     obs_socs[i] = 0;
                     break;
                  }
               }
            }
            if ((tempSocket = accept(sd2, (struct sockaddr *)&cad, &alen)) < 0){
               printf("Error: Accept Failed\n");
               exit(EXIT_FAILURE);
            }

            isConnected[tempSocket] = 1;
            printf("observer socket descriptor = %d, ip addr = %d, port = %d\n", tempSocket,
                     inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

            for (i = 0; i < maxObs; i++){
               if (obs_socs[i] == 0){
                  obs_socs[i] = tempSocket;
                  printf("added socket to list as observer %d\n", i);
                  //send game board message to observer
                  //send(obs_socs[i], &gameMessage, sizeof(gameMessage), 0);
                  break;
               }
            }
            //already full on observers
            if (obs_socs[i] == 0){
               close(tempSocket);
            }
         }
      }
      //---------------------------------------------------------------------------------------
      //----------------------------------- ROUND IS OVER -------------------------------------
      //---------------------------------------------------------------------------------------

      //copy scores to temp array and count how many participants have played
      played = 0;
      for (i=0; i<infoSize; i++){
         tempScores[i] = scores[i];
         if (strcmp(names[i], "") != 0)
            played++;
      }

      //calculate top 5 scores
      for (i=0; i<5; i++){
         highScore = -99999;
         highIndex = 0;
         for (j=0; j<infoSize; j++){
            if (tempScores[j] > highScore && strcmp(names[j], "") != 0){
               highScore = tempScores[j] + 0;
               highIndex = j;
            }
         }
         tempScores[highIndex] = -99999;
         top5[i] = highIndex;
      }
      printf("Round has ended. New round in %d seconds...\n", delayTime);
      //send formatted leaderboard to observers
      for (i=0; i<maxObs; i++){
         if (isConnected[obs_socs[i]]){
            if (played == 1) {
               snprintf(gameMessage, sizeof(gameMessage), "Leaderboard:\n"
                                                          "1) %s has %d points\n",
                                                          names[top5[0]], scores[top5[0]]);
               send(obs_socs[i], &gameMessage, strlen(gameMessage), 0);
            }else if (played == 2) {
               snprintf(gameMessage, sizeof(gameMessage), "Leaderboard:\n"
                                                          "1) %s has %d points\n"
                                                          "2) %s has %d points\n",
                                                          names[top5[0]], scores[top5[0]], names[top5[1]], scores[top5[1]]);
               send(obs_socs[i], &gameMessage, strlen(gameMessage), 0);
            }else if (played == 3) {
               snprintf(gameMessage, sizeof(gameMessage), "Leaderboard:\n"
                                                          "1) %s has %d points\n"
                                                          "2) %s has %d points\n"
                                                          "3) %s has %d points\n",
                                                          names[top5[0]], scores[top5[0]], names[top5[1]], scores[top5[1]], names[top5[2]], scores[top5[2]]);
               send(obs_socs[i], &gameMessage, strlen(gameMessage), 0);
            }else if (played == 4) {
               snprintf(gameMessage, sizeof(gameMessage), "Leaderboard:\n"
                                                          "1) %s has %d points\n"
                                                          "2) %s has %d points\n"
                                                          "3) %s has %d points\n"
                                                          "4) %s has %d points\n",
                                                          names[top5[0]], scores[top5[0]], names[top5[1]], scores[top5[1]], names[top5[2]], scores[top5[2]], names[top5[3]], scores[top5[3]]);
               send(obs_socs[i], &gameMessage, strlen(gameMessage), 0);
            }
            else if (played >= 5) {
               snprintf(gameMessage, sizeof(gameMessage), "Leaderboard:\n"
                                                          "1) %s has %d points\n"
                                                          "2) %s has %d points\n"
                                                          "3) %s has %d points\n"
                                                          "4) %s has %d points\n"
                                                          "5) %s has %d points\n",
                                                          names[top5[0]], scores[top5[0]], names[top5[1]], scores[top5[1]], names[top5[2]], scores[top5[2]], names[top5[3]], scores[top5[3]], names[top5[4]], scores[top5[4]]);
               send(obs_socs[i], &gameMessage, strlen(gameMessage), 0);
            }
         }
      }

      start = time(NULL);

      while ((int)time(NULL) - (int)start < delayTime){

         //add socket descriptors to set readSet
         FD_ZERO(&readSet);
         FD_SET(sd1, &readSet);
         FD_SET(sd2, &readSet);
         maxSD = sd2;

         //add valid participant socket descriptors to both sets
         for (i = 0; i < infoSize; i++){
            if (par_socs[i] > 0)
               FD_SET(par_socs[i], &writeSet);
               FD_SET(par_socs[i], &readSet);
            //for select function
            if (par_socs[i] > maxSD)
               maxSD = par_socs[i];
         }
         //add valid observer socket descriptors to both sets
         for (i = 0; i < maxObs; i++){
            if (obs_socs[i] > 0)
               FD_SET(obs_socs[i], &writeSet);
               FD_SET(obs_socs[i], &readSet);
            //for select function
            if (obs_socs[i] > maxSD)
               maxSD = obs_socs[i];
         }

         //check for activity on sockets list
         sel = select(maxSD+1, &readSet, &writeSet, NULL, 0);//&roundTime);

         if (sel < 0)
            perror("select()");

         //check if participant disconnected
         for (i=0; i<infoSize; i++) {
            if (isConnected[par_socs[i]] && FD_ISSET(par_socs[i], &readSet)) {
               if (n = recv(par_socs[i], &c, 1, MSG_PEEK) <= 0) {
                  printf("Player %s has left the game.\n", names[i]);
                  isConnected[par_socs[i]] = 0;
                  parCount--;
                  FD_CLR(par_socs[i], &readSet);
                  FD_CLR(par_socs[i], &writeSet);
               }
            }
         }

         //check if observer disconnected
         for (i=0; i<maxObs; i++) {
            if (isConnected[obs_socs[i]] && FD_ISSET(obs_socs[i], &readSet)) {
               if (n = recv(obs_socs[i], &c, 1, 0) <= 0) {
                  printf("Observer disconnected.\n");
                  isConnected[obs_socs[i]] = 0;
                  FD_CLR(obs_socs[i], &readSet);
                  FD_CLR(obs_socs[i], &writeSet);
               }
            }
         }

         //check for participant names sent
         for (i=0; i<infoSize; i++){
            if (isConnected[par_socs[i]] && FD_ISSET(par_socs[i], &readSet) && strcmp(names[i], "") == 0){
               recv(par_socs[i], tempname, sizeof(tempname),0);
               validName = 'Y';
               //check if name is valid, otherwise re-prompt
               if (strlen(tempname) > 0 && strlen(tempname) <= 30){
                  for (j = 0; j < infoSize; j++){
                     if (strcmp(names[j], tempname) == 0){ //name already in use
                        validName = 'N';
                        printf("Invalid name entered.\n");
                        break;
                     }
                  }
                  for (j = 0; j < strlen(tempname); j++){
                     if (!((tempname[j] >= 'a' && tempname[j] <= 'z') ||
                              (tempname[j] >= 'A' && tempname[j] <= 'Z') ||
                              (tempname[j] >= '0' && tempname[j] <= '9') ||
                              tempname[j] == ' ' || tempname[j] == '_')){
                        validName = 'N';
                        break;
                     }
                  }
               }else{
                  validName = 'N';
                  printf("Invalid name entered.\n");
               }
               send(par_socs[i], &validName, sizeof(validName), 0);
               if (validName == 'Y'){
                  names[i] = strdup(tempname);
                  printf("Player has chosen the name, \"%s\"\n", tempname);
               }
            //check for moves entered by participants
         }else if(isConnected[par_socs[i]] && FD_ISSET(par_socs[i], &readSet) && strcmp(names[i], "") != 0){
               //get move from player, but do nothing with it
               recv(par_socs[i], playerMove, sizeof(playerMove), 0);
            }
         }

         //check if participant connected
         if (FD_ISSET(sd1, &readSet)){
            //check if another socket needs to be taken over
            gotSD = 0;
            for (i=0; i<infoSize; i++){
               if (isConnected[par_socs[i]] == 0 && par_socs[i] != 0){
                  close(par_socs[i]);
                  par_socs[i] = 0;
                  gotSD = 1;
                  break;
               }
            }
            if (!gotSD){
               for (i=0; i<maxObs; i++){
                  if (isConnected[obs_socs[i]] == 0 && obs_socs[i] != 0) {
                     close(obs_socs[i]);
                     obs_socs[i] = 0;
                     break;
                  }
               }
            }
            if ((tempSocket = accept(sd1, (struct sockaddr *)&cad, &alen)) < 0){
               printf("Error: Accept Failed\n");
               exit(EXIT_FAILURE);
            }

            isConnected[tempSocket] = 1;
            printf("participant socket descriptor = %d, ip addr = %d, port = %d\n", tempSocket,
                     inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

            if (parCount < maxPar){
               //check for an empty spot to reserve for participant
               spotFound = 0;
               for (i = 0; i < infoSize; i++){
                  if (strcmp(names[i], "") == 0){
                     par_socs[i] = tempSocket;
                     parCount++;
                     printf("added socket to list as participant %d\n", i);
                     spotFound = 1;
                     break;
                  }
               }
               //have to kick an old player
               //find low score so they can be kicked
               if (!spotFound){
                  lowScore = 0;
                  for (i = 0; i < infoSize; i++){
                     if (i < lowScore)
                        lowScore = i;
                  }
                  //check for a spot to steal for participant (low score)
                  for (i = 0; i < infoSize; i++){
                     if (par_socs[i] == 0 && scores[i] == lowScore){
                        par_socs[i] = tempSocket;
                        names[i] = "";
                        parCount++;
                        printf("added socket to list as participant %d\n", i);
                        break;
                     }
                  }
               }
            }
            //already full on participants
            else
               close(tempSocket);
         }

         //check if observer connected
         if (FD_ISSET(sd2, &readSet)){
            //check if another socket needs to be taken over
            gotSD = 0;
            for (i=0; i<infoSize; i++){
               if (isConnected[par_socs[i]] == 0 && par_socs[i] != 0){
                  close(par_socs[i]);
                  par_socs[i] = 0;
                  gotSD = 1;
                  break;
               }
            }
            if (!gotSD){
               for (i=0; i<maxObs; i++){
                  if (isConnected[obs_socs[i]] == 0 && obs_socs[i] != 0) {
                     close(obs_socs[i]);
                     obs_socs[i] = 0;
                     break;
                  }
               }
            }
            if ((tempSocket = accept(sd2, (struct sockaddr *)&cad, &alen)) < 0){
               printf("Error: Accept Failed\n");
               exit(EXIT_FAILURE);
            }

            isConnected[tempSocket] = 1;
            printf("observer socket descriptor = %d, ip addr = %d, port = %d\n", tempSocket,
                     inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

            for (i = 0; i < maxObs; i++){
               if (obs_socs[i] == 0){
                  obs_socs[i] = tempSocket;
                  printf("added socket to list as observer %d\n", i);
                  //send game board message to observer
                  //send(obs_socs[i], &gameMessage, sizeof(gameMessage), 0);
                  break;
               }
            }
            //already full on observers
            if (obs_socs[i] == 0){
               close(tempSocket);
            }
         }
      }

      //---------------------------------------------------------------------------------------
      //----------------------------------- DELAY IS OVER -------------------------------------
      //---------------------------------------------------------------------------------------

      //reset game board values for new round;
      for (i=0; i<81; i++)
         gb[i] = '0';

      printf("New Round has begun. You have %d seconds.\n", roundTime);
      for (i=0; i<maxObs; i++){
         if (isConnected[obs_socs[i]]){
            snprintf(gameMessage, sizeof(gameMessage), "New Round has begun. You have %d seconds.\n", roundTime);
            send(obs_socs[i], &gameMessage, strlen(gameMessage), 0);
         }
      }
   }
}
