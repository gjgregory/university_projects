/*
* Command-Line Calculator
* Garrett Gregory, 11 May 2016
*
* This is a calculator that performs basic operations (+ - * /) between two integers. To
* exit, type 'exit' or press Ctrl-D.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "gg_cli.h"
#define MAX_INPUTLEN 2048


/* getOp()
 * this function parses a string and returns the first operator character
 * it finds. if none are found, return 0;
 */
int getOp(char *input, char *symbol){
   int i;

   for (i=1; i<strlen(input)-1; i++){ //start at index 1 to ignore negative first num
      if (input[i] == '*' || input[i] == '/' || input[i] == '+' || input[i] == '-'){
         *symbol = input[i];
         input[i] = ' ';
         return 1;
      }
   }
   return 0;
}


/* isCalc()
 * this function determines whether or not the input is a viable calc command.
 */
int isCalc(char **input){
   int i, j;

   //check if too few or too many args
   if (input[0] == NULL || input[1] == NULL || input[2] != NULL) return 0;

   for (i=0; i <= 1; i++){
      for (j=0; j < strlen(input[i]); j++){
         //non-numeric char (ok if negative sign)
         if (!isdigit(input[i][j]) && input[i][j] != 0 && !(input[i][0] == '-' && !j)) return 0;
      }
   }

   return 1;
}


/* doCalc()
 * this function performs and outputs the calculation specified
 * correctly by the user.
 */
void doCalc(char **input, char symbol){
   switch(symbol){
      case '+':
         printf("%s + %s = %d\n", input[0], input[1], atoi(input[0])+atoi(input[1]));
         break;
      case '-':
         printf("%s - %s = %d\n", input[0], input[1], atoi(input[0])-atoi(input[1]));
         break;
      case '*':
         printf("%s * %s = %d\n", input[0], input[1], atoi(input[0])*atoi(input[1]));
         break;
      case '/':
         if (atoi(input[1]) == 0)
            printf("Cannot divide by zero!\n");
         else
            printf("%s / %s = %d\n", input[0], input[1], atoi(input[0])/atoi(input[1]));
         break;
   }
}


/* getCalcInput()
 * this is the main driver function for the calc program.
 */
void getCalcInput(){
   char *user_input = malloc(MAX_INPUTLEN);
   char **idx_input = NULL;
   int n=0;
   char mathSymbol = ' '; //used to determine math calculation type (if any)
   char *line;

   line = fgets(user_input, MAX_INPUTLEN, stdin);
   user_input[strlen(user_input)-1] = 0;
   //check for exit commands
   if (line == NULL || strcmp(user_input, "exit") == 0)
      exit(0);

   //check if input is a math calculation
   if (getOp(user_input, &mathSymbol)){
      idx_input = towkanize(user_input, &n);
      if (isCalc(idx_input)){
         doCalc(idx_input, mathSymbol);
         free(user_input);
         free(idx_input);
         return;
      }
   free(user_input);
   free(idx_input);
   }
   printf("Invalid operation! Must be a single 2-number operation.\n");
}
