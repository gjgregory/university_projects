/* Josh Richmond && Garrett Gregory, 2 December 2016
 *
 * CSCI 460 Assignment 4
 * File System API
 *
 *
 * This .c file implements functions declared in FileSysAPI.h that are used by the
 * provided tests in order to simulate a file system using modified i-nodes, a
 * single-level directory, and a linked list of free blocks.
 *
 *
 * ***NOTE: Print statements can be toggled on/off by changing the DEBUG constant.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Driver.h"
#include "FileSysAPI.h"

#define DEBUG 0
#define BLOCK_SIZE 64 // Bytes of data per block
#define INDR_BLOCKS 16 // Degree of indirection for logical blocks
#define MAX_FILE_SIZE 4381

// Prototypes
int get_next_block();
int write_data(int logical_block, int data_left, int index, char* data);
struct file *find_file(char *file_name);
int add_free_block(int block);

// File node used by single-level directory linked list
struct file {
	char* name;    // Name of file
	int size;    // Size of file
	int block;   // Block number where data for file begins, location to i-node
	struct file* next;    // Pointer to next file on system
};

// I-node structure for files
struct block{
	int logical_blocks[INDR_BLOCKS]; // Each i-node contains 16 logical block numbers
};

// Linked list of free files locations
struct free_node {
	int block;    // Locations of first free block
	int span;    // Number of free blocks in this section
	struct free_node* next;    // Pointer to next free block section
};

struct file *head;
struct file *tail;
int blocks_remaining = SECTORS;
struct free_node *free_list;	// Head of linked list of free nodes
struct block *blocks[SECTORS];    // Array containing all i-nodes

/* CSCI460_Format()
 *
 * This function formats/initializes the file system prior to Performing
 * reads and writes.
 */
int CSCI460_Format(){
	if(DevFormat()) {    // Attempt to create file system
		head = (struct file*)malloc(sizeof(struct file));    // Allocate pointer to flat directory
		tail = head;
		struct block *temp;
		// Allocate memory for entire file system
		// Create SECTORS number of i_nodes
		for(int i = 0; i < SECTORS; i++){
			for(int j = 0; j < INDR_BLOCKS; j++){
				temp = (struct block*)malloc(sizeof(struct block));
				blocks[i] = temp;
				blocks[i]->logical_blocks[j] = -1;    // Set blocks negative->
			}
		}
		// Create the first free node
		// contains the first block and all blocks of filesystem
		free_list = (struct free_node*)malloc(sizeof(struct free_node));
		free_list->block = 0;
		free_list->span = SECTORS;
		free_list->next = NULL;
		return 1;
	}
	return 0;
}

/* CSCI460_Write()
 *
 * This function writes the string Data to a new file with the specified filename.
 * If a file with that name already exists, it will be deleted, then rewritten as
 * a new file.
 */
int CSCI460_Write(char *fileName, int size, char *Data){
	int count = 0;
	int index = 0;
	int next_block;
	int data_left;
	int i_node_index;
	int num_blocks = size / BLOCK_SIZE + 1;    // The number of nodes needed for this file
	struct file *new_file;
	struct block *i_node;

	// If file already exists, delete that file
	if(find_file(fileName) != NULL){
		if (DEBUG) printf("file already exists. deleting...\n");
		CSCI460_Delete(fileName);
	}

	i_node_index = get_next_block();
	new_file = (struct file*)malloc(sizeof(struct file));
	new_file->name = fileName;
	new_file->size = size;
	new_file->block = i_node_index;
	new_file->next = NULL;

	tail->next = new_file;
	tail = new_file;

	i_node = blocks[i_node_index];
	// Check to see if we have enough room
	if (DEBUG) printf("blocks needed: %d, blocks remaining: %d\n",num_blocks, blocks_remaining);
	if (blocks_remaining < num_blocks){
		if (DEBUG)printf("Error: not enough space in filesystem for %s with size %d",fileName,size);
		return 0;
	}

	// Direct blocks
	for(int i = 0; i < 13 && count < num_blocks; i++){
		next_block = get_next_block();
		i_node->logical_blocks[i] = next_block;
		count++;

		data_left = strlen(Data) - index;
		if(!write_data(next_block, data_left, index, Data)){
			return 0;
		}

		if(data_left > BLOCK_SIZE){
			index += BLOCK_SIZE;
		}else{
			index += data_left;
		}
	}

	// First level of indirection
	i_node->logical_blocks[13] = get_next_block();
	for(int i = 0; i < INDR_BLOCKS && count < num_blocks; i++){
		next_block = get_next_block();
		blocks[i_node->logical_blocks[13]]->logical_blocks[i] = next_block;
		count++;

		data_left = strlen(Data) - index;
		if(!write_data(next_block, data_left, index, Data)){
			return 0;
		}

		if(data_left > BLOCK_SIZE){
			index += BLOCK_SIZE;
		}else{
			index += data_left;
		}

	}

	// Second level of indirection
	i_node->logical_blocks[14] = get_next_block();
	for(int i = 0; i < INDR_BLOCKS; i++){
		// Loop through 16 indirect blocks
		blocks[i_node->logical_blocks[14]]->logical_blocks[i] = get_next_block();
		for(int j = 0; j < INDR_BLOCKS && count < num_blocks; j++){
			next_block = get_next_block();
			blocks[blocks[i_node->logical_blocks[14]]->logical_blocks[i]]->logical_blocks[j] = next_block;
			count++;

			data_left = strlen(Data) - index;
			if(!write_data(next_block, data_left, index, Data)){
				return 0;
			}

			if(data_left > BLOCK_SIZE){
				index += BLOCK_SIZE;
			}else{
				index += data_left;
			}
		}
	}

	// Third level of indirection
	i_node->logical_blocks[15] = get_next_block();
	for(int i = 0; i < INDR_BLOCKS; i++){
		// loop through first indirect
		blocks[i_node->logical_blocks[15]]->logical_blocks[i] = get_next_block();
		for(int j = 0; j < INDR_BLOCKS; j++){
			// loop through second indirect
			blocks[blocks[i_node->logical_blocks[15]]->logical_blocks[i]]->logical_blocks[j] = get_next_block();
			for(int k = 0; k < INDR_BLOCKS && count < num_blocks; k++){
				next_block = get_next_block();
				blocks[blocks[blocks[i_node->logical_blocks[15]]->logical_blocks[i]]->logical_blocks[j]]->logical_blocks[k] = next_block;
				count++;

				data_left = strlen(Data) - index;
				if(!write_data(next_block, data_left, index, Data)){
					return 0;
				}

				if(data_left > BLOCK_SIZE){
					index += BLOCK_SIZE;
				}else{
					index += data_left;
				}
			}
		}
	}
	return 1;
}

/* CSCI460_Read()
 *
 * This function reads from the file with the specified filename (if it exists), storing
 * the file's contents in the string data.
 */
int CSCI460_Read(char *fileName, int maxSize, char *data){  // Reads entire file
	int count = 0;
	struct file *current_file = find_file(fileName);
	int i_node_index;
	int size;
	int num_blocks;
	char temp[BLOCK_SIZE];
	char output[maxSize];
	int index = 0;

	// If file doesn't exist, return error code 0
	if(current_file == NULL){
		if(DEBUG)printf("Error: Could not find file for reading '%s'\n",fileName);
		return 0;
	}

	i_node_index = current_file->block;
	size = current_file->size;
	num_blocks = size / BLOCK_SIZE + 1;
	if (DEBUG) printf("File found for reading...\n");
	if (DEBUG)printf("i_node_index: %d\n",i_node_index);

	// Direct blocks
	for(int i = 0; i < 13 && count < num_blocks; i++){
		if(!DevRead(blocks[i_node_index]->logical_blocks[i], temp))return 0;
		temp[BLOCK_SIZE] = '\0';
		for(int l = 0; l < (int)strlen(temp); l++){
			output[index] = temp[l];
			index++;
		}
		count++;
	}

	// First level of indirection
	for(int i = 0; i < INDR_BLOCKS && count < num_blocks; i++){
		if(!DevRead(blocks[blocks[i_node_index]->logical_blocks[13]]->logical_blocks[i], temp))return 0;
		temp[BLOCK_SIZE] = '\0';
		for(int l = 0; l < (int)strlen(temp); l++){
			output[index] = temp[l];
			index++;
		}
		count++;
	}

	// Second level of indirection
	for(int i = 0; i < INDR_BLOCKS; i++){
		for(int j = 0; j < INDR_BLOCKS && count < num_blocks; j++){
			if(!DevRead(blocks[blocks[blocks[i_node_index]->logical_blocks[14]]->logical_blocks[i]]->logical_blocks[j], temp))return 0;
			temp[BLOCK_SIZE] = '\0';
			if (DEBUG)printf("block: %d,temp: %s\n",blocks[blocks[blocks[i_node_index]->logical_blocks[14]]->logical_blocks[i]]->logical_blocks[j],temp);
			for(int l = 0; l < (int)strlen(temp); l++){
				output[index] = temp[l];
				index++;
			}
			count++;
		}
	}

	// Third level of indirection
	for(int i = 0; i < INDR_BLOCKS; i++){
		for(int j = 0; j < INDR_BLOCKS; j++){
			for(int k = 0; k < INDR_BLOCKS && count < num_blocks; k++){
				if(!DevRead(blocks[blocks[blocks[blocks[i_node_index]->logical_blocks[15]]->logical_blocks[i]]->logical_blocks[j]]->logical_blocks[k], temp))return 0;
				if (DEBUG)printf("block: %d,temp: %s\n",blocks[blocks[blocks[blocks[i_node_index]->logical_blocks[15]]->logical_blocks[i]]->logical_blocks[j]]->logical_blocks[k],temp);
				temp[BLOCK_SIZE] = '\0';
				for(int l = 0; l < (int)strlen(temp); l++){
					output[index] = temp[l];
					index++;
				}
				count++;
			}
		}
	}
	output[size] = '\0';
	memcpy(data,output,strlen(output)+1);
	return 1;
}

/* CSCI460_Delete()
 *
 * This function finds a file with the provided filename, then deletes it if it exists,
 * adding all blocks in the file back to the linked list of free spans of blocks.
 */
int CSCI460_Delete(char *fileName){
	int count = 0;
	struct file *current_file = find_file(fileName);
	int i_node_index = current_file->block;
	int size = current_file->size;
	int num_blocks = size / BLOCK_SIZE + 1;
	struct file *temp = head;
	struct file *temp2;

	// If file doesn't exist, return error code 0
	if(current_file == NULL){
		if(DEBUG)printf("Error: Could not find file for deletion '%s'\n",fileName);
		return 0;
	}
	if (DEBUG) printf("File found for deletion...\n");

	// Add all blocks to free list...

	// Direct blocks
	for(int i = 0; i < 13 && count < num_blocks; i++){
		add_free_block(blocks[i_node_index]->logical_blocks[i]);
		count++;
	}

	// First level of indirection
	for(int i = 0; i < INDR_BLOCKS && count < num_blocks; i++){
		add_free_block(blocks[blocks[i_node_index]->logical_blocks[13]]->logical_blocks[i]);
		count++;
	}
	add_free_block(blocks[i_node_index]->logical_blocks[13]);

	// Second level of indirection
	for(int i = 0; i < INDR_BLOCKS; i++){
		for(int j = 0; j < INDR_BLOCKS && count < num_blocks; j++){
			add_free_block(blocks[blocks[blocks[i_node_index]->logical_blocks[14]]->logical_blocks[i]]->logical_blocks[j]);
			count++;
		}
		add_free_block(blocks[blocks[i_node_index]->logical_blocks[14]]->logical_blocks[i]);
	}
	add_free_block(blocks[i_node_index]->logical_blocks[14]);

	// Third level of indirection
	for(int i = 0; i < INDR_BLOCKS; i++){
		for(int j = 0; j < INDR_BLOCKS; j++){
			for(int k = 0; k < INDR_BLOCKS && count < num_blocks; k++){
				add_free_block(blocks[blocks[blocks[blocks[i_node_index]->logical_blocks[15]]->logical_blocks[i]]->logical_blocks[j]]->logical_blocks[k]);
				count++;
			}
			add_free_block(blocks[blocks[blocks[i_node_index]->logical_blocks[15]]->logical_blocks[i]]->logical_blocks[j]);

		}
		add_free_block(blocks[blocks[i_node_index]->logical_blocks[15]]->logical_blocks[i]);
	}
	add_free_block(blocks[i_node_index]->logical_blocks[15]);

	// Find file in directory list, then remove it
	while (temp->next != current_file){
		temp = temp->next;
		if(temp->next == current_file){
			temp2 = temp->next;
			temp->next = temp->next->next;
			free(temp2);
			break;
		}
	}
	return 1;
}

/* add_free_block()
 *
 * This function adds a block to the free list by combining it with one or two spans
 * adjacent to it, or starting a new span of its own.
 */
int add_free_block(int block){
	struct free_node *front = free_list;
	struct free_node *rear = NULL;
	struct free_node *temp;
	blocks_remaining++;

	// Find an appropriately ordered spot in free list
	while (front != NULL){
		// block is one index lower than front node
		if(block == front->block-1){
			front->block--;
			front->span++;
			if (rear != NULL){
				// block is one index higher than rear node
				if(front->block == rear->block + rear->span){
					rear->span += front->span;
					rear->next = front->next;
					free(front);
				}
			}
			return 1;
		}
		if (rear != NULL){
			// block is one index higher than rear node
			if (block == rear->block + rear->span){
				rear->span++;
				return 1;
			}
		}
		// block is lower than front node, but not in its span
		if (block < front->block -1){
			temp = (struct free_node*)malloc(sizeof(struct free_node));
			temp->block = block;
			temp->span = 1;
			temp->next = front;
			if (rear == NULL){
				free_list = temp;
				return 1;
			}
			// block is higher than rear node, but not in its span
			if (block > rear->block + rear->span){
				rear->next = temp;
				return 1;
			}
		}
		// move front/rear both one node forward
		rear = front;
		front = front->next;
	}
	// Error (this should never happen)
	return 0;
}

/* find_file()
 *
 * This function finds a file in the directory, then returns a pointer to that file
 * struct. If no file is found, it will instead return NULL, indicating an error.
 */
struct file *find_file(char* file_name){
	if(DEBUG)printf("looking for file %s\n",file_name);
	struct file *temp = head;
	while (temp != NULL){
		if (DEBUG)printf("find_file temp->name = %s\n",temp->name);
		if(temp->name != NULL){
			if(strcmp(temp->name,file_name) == 0){
				if(DEBUG)printf("found file\n");
				return temp;
			}
		}
		if(DEBUG)printf("didn't find file\n");
		temp = temp->next;
		if(DEBUG)printf("after temp = temp->next\n");
	}
	if(DEBUG)printf("file doesn't exist\n");
	return NULL;
}

/* write_data()
 *
 * This function writes up to BLOCK_SIZE bytes of data at the specified index in
 * the string data to a single block at index logical_block. If DevWrite
 * returns 0, indicating an error, so will write_data.
 */
int write_data(int logical_block, int data_left, int index, char* data){
	char temp[BLOCK_SIZE];
	if(data_left > BLOCK_SIZE){
		strncpy(temp, data+index, BLOCK_SIZE);
		temp[BLOCK_SIZE] = '\0';
	}else{
		strncpy(temp, data+index, data_left);
		temp[data_left] = '\0';
	}
	if (DEBUG)printf("data written : '%s'\n",temp);
	if(!DevWrite(logical_block,temp)){
		return 0;
	}
	return 1;
}


/* CSCI460_Delete()
 *
 * This function finds the first block in any existing spans in the free list,
 * returns the index of that one block. If the free list is empty, it instead
 * returns -1, indicating an error.
 */
int get_next_block(){
	struct free_node *temp;

	if (free_list == NULL){
		if (DEBUG) printf("YOU RAN OUTA BLOCKS!\n");
		return -1;
	}
	int output = free_list->block;    // Our return value is the first block number
	free_list->span--;    // Decrement our span
	if(free_list->span == 0){
		temp = free_list;
		free_list = free_list->next;    // Remove element from list if span is now empty
		free(temp);
	}else{
		free_list->block++;    // Make the next block in span the first block
	}
	blocks_remaining--;    // Decrement total free blocks in file_system
	return output;
}
