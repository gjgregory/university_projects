/*
* CSCI 352 Assignment 2 - Command-Line Interpreter
* Garrett Gregory, 11 May 2016
*
* This is a custom command-line interpreter that uses its own commands, while also
* borrowing standard bash functionality so as to not lose any functionality. It also still
* supports nested I/O redirection.
*
* Custom commands:
*
*       calc --- starts a calculator process. performs 2-number integer calculations. exit with 'exit' or Ctrl-D
*       pwd --- prints the path of the current working directory
*       cd [path]--- changes the path of the current working directory (relative or static)
*       listf --- lists the contents of current working directory, similarly to 'ls'. available options: l, a, c, m
*               (options a, c, and m all require option l to be specified)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "gg_calc.h"
#include "gg_dir.h"
#define MAX_INPUTLEN 2048
#define MAX_DIR 1024


/* towkanize()
 * tokenizer function. this splits up sub-strings of a string
 * using strtok(), using white-space as a delimiter.
 */
char **towkanize(char *input, int *n){
   char **idx_input = NULL;
   char *split_input = strtok(input, " \n");

   while (split_input) {
      idx_input = realloc(idx_input, sizeof(char*) * ++*n);

      if (idx_input == NULL)
         exit(-1);

      idx_input[*n-1] = split_input;
      split_input = strtok(NULL, " \n");
   }

   idx_input = realloc(idx_input, sizeof(char*) * (*n+1));
   idx_input[*n] = '\0';

   return idx_input;
}


/* check_inputRedir()
 * This function checks whether the user specified to redirect the input, and determines
 * where to if necessary.
 */
int check_inputRedir(char **input){
   char filename[MAX_DIR];
   char in_cpy[strlen(*input)];
   int i, j, startIdx=0, endIdx, cpysize;

   memcpy(in_cpy, &*input[0], strlen(*input));
   in_cpy[strlen(*input)] = '\0';

   //find '>'
   for (i=0; i < strlen(in_cpy); i++){
      if (in_cpy[i] == '<'){
         endIdx = 0;
         //find start of filename
         for (j=i+1; j < strlen(in_cpy); j++){
            if (in_cpy[j] == '>' || in_cpy[j] == '<') return 0; // no file specified
            if (in_cpy[j] != ' '){
               startIdx = j;
               endIdx = j;
               //find end of filename
               while (in_cpy[endIdx] != ' ' && endIdx < (int)strlen(in_cpy) &&
                        in_cpy[endIdx] != '>' && in_cpy[endIdx] != '<'){
                  endIdx++;
               }
               break;
            }
         }
         if (endIdx == 0) return 0; // no file specified

         // set redirection filename
         memmove(&filename[0], &in_cpy[startIdx], endIdx-startIdx);
         filename[endIdx-startIdx]='\0';
         // cut output redirection stuff out of string
         cpysize = (int)strlen(in_cpy)-endIdx + 1;
         memmove(&in_cpy[i], &in_cpy[endIdx], cpysize);
         in_cpy[cpysize+i+1]='\0';

         i--; // make sure not to skip a byte
      }
   }

   if (startIdx == 0) return 1; // no '<' was found. do nothing.

   if (freopen(filename, "r", stdin) == NULL)
      fprintf(stderr, "Error: cannot access filename '%s'\n", filename);

   strcpy(*input, in_cpy);

   return 1;
}


/* check_outputRedir()
 * This function checks whether the user specified to redirect the output, and determines
 * where to if necessary.
 */
int check_outputRedir(char **input){
   char filename[MAX_DIR];
   char in_cpy[strlen(*input)];
   int i, j, startIdx=0, endIdx, cpysize;

   memcpy(in_cpy, &*input[0], strlen(*input));
   in_cpy[strlen(*input)] = '\0';

   //find '>'
   for (i=0; i < strlen(in_cpy); i++){
      if (in_cpy[i] == '>'){
         endIdx = 0;
         //find start of filename
         for (j=i+1; j < strlen(in_cpy); j++){
            if (in_cpy[j] == '>' || in_cpy[j] == '<') return 0; // no file specified
            if (in_cpy[j] != ' '){
               startIdx = j;
               endIdx = j;
               //find end of filename
               while (in_cpy[endIdx] != ' ' && endIdx < (int)strlen(in_cpy) &&
                        in_cpy[endIdx] != '>' && in_cpy[endIdx] != '<'){
                  endIdx++;
               }
               break;
            }
         }
         if (endIdx == 0) return 0; // no file specified

         // set redirection filename
         memmove(&filename[0], &in_cpy[startIdx], endIdx-startIdx);
         filename[endIdx-startIdx]='\0';
         // cut output redirection stuff out of string
         cpysize = (int)strlen(in_cpy)-endIdx + 1;
         memmove(&in_cpy[i], &in_cpy[endIdx], cpysize);
         in_cpy[cpysize+i+1]='\0';

         i--; // make sure not to skip a byte
      }
   }

   if (startIdx == 0) return 1; // no '>' was found. do nothing.

   if (freopen(filename, "w", stdout) == NULL)
      fprintf(stderr, "Error: cannot access filename '%s'\n", filename);

   strcpy(*input, in_cpy);

   return 1;
}


/* getInput()
 * this function gets input from the user and decides what do do with it.
 */
void getInput(){
   char *user_input = malloc(MAX_INPUTLEN);
   char **idx_input = NULL;
   int n=0, pid;
   char *line;
   char *temptxt;
   char *pwd;

   printf("$> ");
   line = fgets(user_input, MAX_INPUTLEN, stdin);
   user_input[strlen(user_input)-1] = 0;

   //check for exit commands
   if (line == NULL || strcmp(user_input, "exit") == 0)
      exit(0);

   //check for I/O redirection
   if (!check_inputRedir(&user_input)){
      fprintf(stderr, "Error: no input redirection path specified!\n");
      freopen ("/dev/tty", "r", stdin);
      return;
   }
   if (!check_outputRedir(&user_input)){
      fprintf(stderr, "Error: no output redirection path specified!\n");
      freopen ("/dev/tty", "r", stdin);
      freopen ("/dev/tty", "w", stdout);
      return;
   }

   idx_input = towkanize(user_input, &n);
   temptxt = malloc(strlen(idx_input[0]));

   //no command was specified (possible I/O redirection occurred)
   if (idx_input[0] == NULL){
      freopen ("/dev/tty", "r", stdin);
      freopen ("/dev/tty", "w", stdout);
      return;
   }
   //start calc child process
   if (strcmp(idx_input[0], "calc") == 0){
      printf("child process for calc started...\n");
      pid = fork();
      if (pid == 0){
         while (1){
            getCalcInput();
         }
      }
      else{
         int retStatus;
         waitpid(pid, &retStatus, 0);
      }
   }
   //run pwd command
   else if (strcmp(idx_input[0], "pwd") == 0){
      pwd = getPwd();
      printf("%s\n", pwd);
      free(pwd);
   }
   //run cd dir command
   else if (strcmp(idx_input[0], "cd") == 0){
      cd_dir(idx_input[1]);
   }
   //start listf childprocess
   else if (strcmp(idx_input[0], "listf") == 0){
      pid = fork();
      if (pid == 0){
         do_listf(idx_input);
         exit(0);
      }
      else{
         int retStatus;
         waitpid(pid, &retStatus, 0);
      }
   }
   //start /bin/ command child process
   else{
      pid = fork();
      if (pid == 0){
         strcpy(temptxt, idx_input[0]);
         idx_input[0] = malloc(strlen(temptxt)+5);
         sprintf(idx_input[0], "/bin/%s", temptxt);
         if (execv(idx_input[0], idx_input) < 0) perror("Error");
         exit(0);
      }
      else{
         int retStatus;
         waitpid(pid, &retStatus, 0);
      }
   }
   //reset standard I/O
   freopen ("/dev/tty", "r", stdin);
   freopen ("/dev/tty", "w", stdout);

   free(user_input);
   free(idx_input);
   free(temptxt);
}

int main (int argc, char *argv[]) {

   while (1){
      getInput();
   }

}
