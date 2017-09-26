/*
* Command-Line Directory Commands
* Garrett Gregory, 11 May 2016
*
* This contains functionality for the cd, pwd, and listf commands.
*
* listf options:
*       -l --- outputs directory content info in default list format (required for other options)
*       -a --- includes 'accessed' date of files
*       -c --- includes 'created' date of files
*       -m --- includes 'modified' date of files
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_DIR 1024
#define MAX_INPUTLEN 2048
#define MAX_FILE 512



/* getPwd()
 * this function outputs the current working directory to stdout.
 */
char *getPwd(){
   char *buf = malloc(MAX_DIR);

   buf = getcwd(buf, MAX_DIR);
   if (buf != NULL) return buf;

   printf("error bro can't do it.\n");
   return NULL;
}


/* cd_dir()
 * this function changes the current working directory to a new one
 * specified by the user.
 */
void cd_dir(char *path){
   int success;
   char *pwd;

   success = chdir(path);
   if (success==0){
      pwd = getPwd();
      printf("cwd changed to %s\n", pwd);
      free(pwd);
   }
   else
      fprintf(stderr, "no such directory: %s\n", path);
}


/* ncs_alphasort()
 *
 * this is a modification to alphasort() that ignores case-sensitivity.
 */
int ncs_alphasort(const struct dirent **a, const struct dirent **b) {
   int i;
   char *x = strdup((*a)->d_name);
   char *y = strdup((*b)->d_name);

   // make lowercase (a,b unchanged)
   for (i=0; x[i] != 0; i++) x[i] = tolower(x[i]);
   for (i=0; y[i] != 0; i++) y[i] = tolower(y[i]);

   return strcmp(x,y);
}


/* do_listf()
 *
 * this function performs the listf command, printing file type, permissions,
 * links, owners, groups, sizes, related dates, and file names of all files
 * in the specified directory(s), or the current working directory by default.
 */
void do_listf(char **args){
   char* dirs[MAX_INPUTLEN] = {0};
   DIR* dir;
   struct dirent **entries;
   char filenames[MAX_FILE][MAX_FILE];
   int filecount[MAX_FILE] = {0};
   int i, j, k, z, n;
   int l=0, a=0, c=0, m=0;
   int max_fsize=1, max_digits=0;
   struct stat file_stats;
   char filepath[MAX_DIR];
   char prettypath[MAX_DIR];
   char datestr[16];


   //get flags
   for (i=1; args[i] != NULL; i++){
      if (args[i][0] == '-'){
         for (j=1; args[i][j] != '\0'; j++){
            if (args[i][j] == 'l') l=1;
            else if (args[i][j] == 'a') a=1;
            else if (args[i][j] == 'c') c=1;
            else if (args[i][j] == 'm') m=1;
            else {
               fprintf(stderr, "listf: invalid option -- '%c'\n", args[i][j]);
               return;
            }
         }
      }
   }

   //get directories
   j = 0;
   for (i=1; args[i] != NULL; i++){
      if (args[i][0] != '-'){
         if ((dir = opendir(args[i]))){
            dirs[j] = args[i];
            j++;
            closedir(dir);
         }
         else{
            fprintf(stderr, "listf: cannot access '%s': No such directory\n", args[i]);
         }
      }
   }

   if (!j) dirs[0] = "."; //no directories were specified. use pwd

   //get filenames
   for (i=0; dirs[i] != NULL; i++){
      n = scandir(dirs[i], &entries, NULL, ncs_alphasort);
      if (n<0) perror("scandir");
      else{
         for (j=0; j < n; j++){
            if (!(strcmp(entries[j]->d_name, ".") == 0) && !(strcmp(entries[j]->d_name, "..") == 0)){
               strcpy(filenames[k], entries[j]->d_name);
               filecount[i]++;
               k++;
            }
            free(entries[j]);
         }
         free(entries);
      }
   }

   // formatted output
   j = 0; k = 0;
   for (i=0; dirs[i] != NULL; i++){

      realpath(dirs[i], prettypath);

      z = k;
      k += filecount[i];
      while (z < k){
         snprintf(filepath, MAX_DIR, "%s/%s", prettypath, filenames[z]);
         if(stat(filepath, &file_stats) < 0)
            perror("stat");
         if (max_fsize < (int)file_stats.st_size)
            max_fsize = (int)file_stats.st_size;
         z++;
      }

      // get max filesize digits for printf formatting
      while (max_fsize){
         max_fsize /= 10;
         max_digits++;
      }

      if (dirs[1] != NULL) printf("%s:\n", prettypath);
      printf("%d entries found\n", filecount[i]);
      while (j < k){
         snprintf(filepath, MAX_DIR, "%s/%s", prettypath, filenames[j]);
         if(stat(filepath, &file_stats) < 0)
            perror("stat");

         if (!l){
            printf("%s ", filenames[j]);
         }
         else{
            // file type
            if (S_ISDIR(file_stats.st_mode)) printf("d");
            else if (S_ISREG(file_stats.st_mode)) printf("-");
            else if (S_ISLNK(file_stats.st_mode)) printf("l");
            else printf("o");

            // permissions
            printf((file_stats.st_mode & S_IRUSR) ? "r" : "-");
            printf((file_stats.st_mode & S_IWUSR) ? "w" : "-");
            printf((file_stats.st_mode & S_IXUSR) ? "x" : "-");
            printf((file_stats.st_mode & S_IRGRP) ? "r" : "-");
            printf((file_stats.st_mode & S_IWGRP) ? "w" : "-");
            printf((file_stats.st_mode & S_IXGRP) ? "x" : "-");
            printf((file_stats.st_mode & S_IROTH) ? "r" : "-");
            printf((file_stats.st_mode & S_IWOTH) ? "w" : "-");
            printf((file_stats.st_mode & S_IXOTH) ? "x" : "-");

            // # of links to file
            printf(" %d", (int)file_stats.st_nlink);

            // owner/group names
            printf(" %s", getpwuid(file_stats.st_uid)->pw_name);
            printf(" %s", getgrgid(file_stats.st_gid)->gr_name);

            // size in bytes
            printf(" %*d", max_digits, (int)file_stats.st_size);

            // dates/times
            if (m || (!a && !c)){ // modification
               strftime(datestr, sizeof(datestr), "M%m/%d/%y-%H:%M", localtime(&(file_stats.st_mtime)));
               printf(" %s", datestr);
            }
            if (a){ // access
               strftime(datestr, sizeof(datestr), "A%m/%d/%y-%H:%M", localtime(&(file_stats.st_atime)));
               printf(" %s", datestr);
            }
            if (c){ // creation
               strftime(datestr, sizeof(datestr), "C%m/%d/%y-%H:%M", localtime(&(file_stats.st_ctime)));
               printf(" %s", datestr);
            }

            // file names
            printf(" %s", filenames[j]);
         }

         j++;
         if (j<k) printf("\n");
      }
      printf("\n");
      if (dirs[i+1] != NULL) printf("\n");
      max_fsize = 1;
      max_digits = 0;
   }

   return;
}
