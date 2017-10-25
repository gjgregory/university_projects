/*
 *	Source file APITest.c
 *
 *	Test program for file system API for CSCI 460 Assignment 3
 *
 *	David Bover, WWU, March 2003
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FileSysAPI.h"
#include "Driver.h"


#define TABLE_SIZE	917
#define HASH_SKIP	3
#define MAX_CONTENT	20480

struct data {
	char *name;
	char *content;
	int life;
	struct data *next;
	struct data *prev;
};

FILE *logg;

struct data *table[TABLE_SIZE];

	// test run parameters
	int min_namelength, max_namelength;
	int min_life, max_life;
	int min_content, max_content;
	int iterations;
	char line[50];
	char desc[21];

	int debug = 1;

void GetParameters() {
	FILE *config;
	if (!(config = fopen("Config.txt", "r"))) {
		fprintf(logg, "Cannot read configuration file Config.txt\n");
		exit(0);
	}
	fgets(line, 50, config);
	sscanf(line, "%d", &iterations);
	fgets(line, 50, config);
	sscanf(line, "%d", &min_namelength);
	fgets(line, 50, config);
	sscanf(line, "%d", &max_namelength);
	fgets(line, 50, config);
	sscanf(line, "%d", &min_life);
	fgets(line, 50, config);
	sscanf(line, "%d", &max_life);
	fgets(line, 50, config);
	sscanf(line, "%d", &min_content);
	fgets(line, 50, config);
	sscanf(line, "%d", &max_content);
/*
	fprintf(logg, "Config parameters %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
		cyls, trks, scts, blks,
		iterations,
		min_namelength, max_namelength, min_life, max_life,
		min_content, max_content);
*/
	fclose(config);

}

char *MakeContent() {
	int len = rand() % (max_content - min_content) + min_content;
	char *res = (char *)malloc(len+1);
	for (int i = 0; i < len-1; i++)
		res[i] = rand() % 96 + ' ';
	res[len-1] = '\0';
	return res;
}


char *MakeName() {
	int len = rand() % (max_namelength - min_namelength) + min_namelength;
	char *res = (char *)malloc(len+1);
	for (int i = 0; i < len-1; i++)
		res[i] = rand() % ('z' - 'a') + 'a';
	res[len-1] = '\0';
	return res;
}

void TestFile(struct data *loc) {
	char current[MAX_CONTENT];
	if (!CSCI460_Read(loc->name, MAX_CONTENT, current)) {
		fprintf(logg, "Error reading file %s\n", loc->name);
		exit(0);
	}

	if (strcmp(current, loc->content) != 0) {
		fprintf(logg, "File error on reading file %s\n", loc->name);
		fprintf(logg, "Previous content:\n%s\n", loc->content);
		fprintf(logg, "Current content:\n%s\n", current);
		exit(0);
	}

	// 50% chance of a rewrite
	if (rand() % 2) {
		free(loc->content);
		loc->content = MakeContent();
		//fprintf(logg, "Rewriting %s\n", loc->name);
		if (!CSCI460_Write(loc->name, strlen(loc->content), loc->content)) {
			fprintf(logg, "Error writing file %s\n", loc->name);
			exit(0);
		}

		// Now verify
		if (!CSCI460_Read(loc->name, MAX_CONTENT, current)) {
			fprintf(logg, "Error reading file %s\n", loc->name);
			exit(0);
		}

		if (strcmp(current, loc->content) != 0) {
			fprintf(logg, "File error on reading file %s\n", loc->name);
			fprintf(logg, "Previous content:\n%s\n", loc->content);
			fprintf(logg, "Current content:\n%s\n", current);
			exit(0);
		}

	}

	// Is it time to delete the file?
	loc->life -= 1;
	if (loc->life <= 0) {
		//fprintf(logg, "Delete %s\n", loc->name);
		if (!CSCI460_Delete(loc->name)) {
			fprintf(logg, "Error deleting file %s\n", loc->name);
			exit(0);
		}
		if (loc->prev != NULL)
			loc->prev->next = loc->next;
		if (loc->next != NULL)
			loc->next->prev = loc->prev;
		free(loc);
	}

}

// Hash function
int Hash(char *text) {
	int res = 0;
	int index = 0;
	int len = (int)strlen(text);

	for (int i = 0; i < HASH_SKIP; i++) {
		index = (index + HASH_SKIP) % len;
		res += text[index];
	}
	return res % TABLE_SIZE;
}

// Locate a file entry in the symbol table
struct data *Locate(char *name) {
	int index = Hash(name);
	struct data *curr = table[index];
	while (curr != NULL) {
		if (strcmp(name, curr->name) == 0)
			return curr;
		else
			curr = curr->next;
	}
	return NULL;
}

void AgeFiles() {

	int index;
	struct data *loc, *nextone;
	for (index = 0; index < TABLE_SIZE; index++) {
		loc = table[index];
		while (loc != NULL) {

			// Is it time to delete the file?
			(loc->life)--;
			if (loc->life <= 0) {
				//fprintf(logg, "Delete %s\n", loc->name);

				if (!CSCI460_Delete(loc->name)) {
					fprintf(logg, "Error deleting file %s\n", loc->name);
					exit(0);
				}
				nextone = loc->next;
				if (loc->prev != NULL)
					loc->prev->next = loc->next;
				else
					table[index] = loc->next;
				if (loc->next != NULL)
					loc->next->prev = loc->prev;
				free(loc);
				loc = nextone;
			} else
				loc = loc->next;
		}
	}
}

void VerifyAllFiles() {
	int index;
	struct data *loc;
	char current[MAX_CONTENT];
	for (index = 0; index < TABLE_SIZE; index++) {
		loc = table[index];
		while (loc != NULL) {
			// Check on what was written
			//fprintf(logg, "Checking file %s\n", loc->name);
			if (!CSCI460_Read(loc->name, MAX_CONTENT, current)) {
				fprintf(logg, "Error reading file %s\n", loc->name);
				exit(0);
			}

			if (strcmp(current, loc->content) != 0) {
				fprintf(logg, "File error on reading file %s\n", loc->name);
				fprintf(logg, "Previous content:\n%s\n", loc->content);
				fprintf(logg, "Current content:\n%s\n", current);
				exit(0);
			} else {;
				//fprintf(logg, "Verified file %s %d\n", loc->name, loc->life);
			}

			loc = loc->next;
		}
	}

}

int main() {
	logg = fopen("log.txt", "w");

	// Get the values of the test run parameters
	GetParameters();

	// Initialize the file table
	for (int i = 0; i < TABLE_SIZE; i++)
		table[i] = NULL;

	// Create the file system
	if (!CSCI460_Format()) {
		fprintf(logg, "Failed to create file system\n");
		exit(0);
	}


	// The main iteration loop
	char *name;
	int index;
	struct data *loc;

	printf("Testing file system ");
	fprintf(logg, "Testing file system\n");
	for (int i = 0; i < iterations; i++) {
		if (i % 100 == 0) printf(".");

		VerifyAllFiles();
		AgeFiles();

		// Generate a file name
		name = MakeName();
		//fprintf(logg, "Checking name %s\n", name);
		loc = Locate(name);
		if (loc == NULL) {

			// Create new file
			loc = (struct data *)malloc(sizeof(struct data));
			loc->name = (char *)malloc(strlen(name)+1);
			strcpy(loc->name, name);
			loc->content = MakeContent();
			loc->life = min_life + rand() % (max_life + 1 - min_life);
			index = Hash(loc->name);
			loc->next = table[index];
			if (table[index] != NULL)
				table[index]->prev = loc;
			loc->prev = NULL;
			table[index] = loc;
			fprintf(logg, "Creating new file %s at location %d length %d\n", name, index, (int)strlen(loc->content));
			if (!CSCI460_Write(name, strlen(loc->content), loc->content)) {
				fprintf(logg, "Error writing file %s\n", loc->name);
				exit(0);
			}

			// Check on what was written
			char current[MAX_CONTENT];
			if (!CSCI460_Read(loc->name, MAX_CONTENT, current)) {
				fprintf(logg, "Error reading file %s\n", loc->name);
				exit(0);
			}

			if (strcmp(current, loc->content) != 0) {
				fprintf(logg, "File error on reading file %s\n", loc->name);
				fprintf(logg, "Previous content:\n%s\n", loc->content);
				fprintf(logg, "Current content:\n%s\n", current);
				exit(0);
			} else {;
				//fprintf(logg, "Verified file %s %d\n", loc->name, loc->life);
			}


		}

	}
	printf("\n");
	fprintf(logg, "run completed\n");
	return 1;

}
