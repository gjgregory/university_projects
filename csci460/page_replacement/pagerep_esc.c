/* Garrett Gregory && Josh Richmond, 22 November 2016
 *
 * CSCI 460 Assignment 3
 * Page Replacement Simulator - Enhanced Second Chance
 *
 *
 * This .c file implements functions declared in Assg3.h that are used by the Simumlate()
 * function in such a way as to simulate the Enhanced Second Chance page replacement algorithm
 * with an inverted page table.
 *
 *
 * ***NOTE: Print statements can be toggled on/off by changing the DEBUG constant.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "MemSim.h"

#define DEBUG 		0
#define TABLESIZE	48		// 8mb / 4kb = 2048 table entries
#define MAXPROC		16		// maximum number of processes in the system
#define PAGESIZE	4096		// system page size
#define DEFAULT_PAGE	-999		// negative for blank addresses

// structure for physical pages
struct page_entry {
	int pid;
	int page;
	int reference;
	int dirty;
	struct page_entry *next;
};

struct page_entry *prev;		// pointer to previously used page table entry
int procCount = 0;		// number of processes
int fill_count = 0;		// count for how many table entries get used
// counts for final output statistics:
int r_count = 0; // read without write
int rw_count = 0; // write, then read


/* computeCounts()
 *
 * This function takes in an integer representation of a dirty bit as a parameter
 * and uses it to increment the corresponding counts. Those counts are: how many
 * times a page is read, how many times a page is written out before reading a new
 * one, and the total page faults count (the sum of the first two).
 */
void computeCounts(int dirty){
	if (dirty) rw_count++;
	else r_count++;
}

/* resetReferencePrev()
 *
 * This function takes in the pointer to a page_entry struct, which is the entry that
 * was selected by the enhanced second chance algorithm and had its page replaced. So
 * for all frames between this frame and the previously chosen frame, the reference bits
 * are reset to 0. The new "previously replaced frame" then becomes the one that was passed
 * to this function.
 */
void resetReferencePrev(struct page_entry *newprev){
	struct page_entry *temp = prev;

	temp = prev->next;
	while (temp != newprev){
		temp->reference = 0;
		temp = temp->next;
	}

	prev = newprev;
}

/* find()
 *
 * This function is called by Access() and takes in a pid, address, and write option bits
 * as parameters. It determines if there is room in the inverted page table for a new process.
 * If there is no room, find() will return 0. Otherwise, it will add/replace frames to the table
 * when necessary, increment page fault counts, and return 1.
 */
int find(int pid, int address, int write) {
	int pidfound = 0;
	struct page_entry *temp, *temp2 = NULL;
	int page = address/PAGESIZE;

	// check if pid is in the inverted page table
	temp = prev->next;
	while (1){
		if (temp->pid == pid) pidfound = 1;
		temp = temp->next;
		if (temp==prev->next) break;
	}

	if (!pidfound && procCount >= MAXPROC) return 0; // maxed out on processes
	else if (!pidfound){
		procCount++; // process will be allowed
	}

	// attempt to find same page in page table
	temp = prev->next;
	while (1){
		if (temp->page == page){
			//use this entry
			if (temp->pid != pid){
				if (DEBUG) printf("Page taken from process with pid %d.  pid= %d page= %d\n", temp->pid, pid, page);
				temp->pid = pid;
			}
			else if (temp->dirty == 1){
				if (DEBUG) printf("Page already in table. Performing backing store.  pid= %d page= %d\n", pid, page);
			}
			else {
				if (DEBUG) printf("Page already in table :)  pid= %d page= %d\n", pid, page);
			}
			temp->reference = 1;
			if (write) temp->dirty = 1;
			else temp->dirty = 0;

			return 1;
		}
		temp = temp->next;
		if (temp==prev->next) break;
	}

	//attempt to find unused entry in page table
	temp = prev->next;
	while (1){
		if (temp->pid == 0){
			//use this entry
			if (DEBUG) printf("New entry into page table.  pid= %d page= %d\n", pid, page);
			fill_count++;
			computeCounts(temp->dirty);
			temp->pid = pid;
			temp->page = page;
			temp->reference = 0;
			if (write) temp->dirty = 1;
			else temp->dirty = 0;
			//resetReferencePrev(temp);
			return 1;
		}
		temp = temp->next;
		if (temp==prev->next) break;
	}

	//attempt to find entry with reference bit = 0, dirty bit = 0
	temp = prev->next;
	while (1){
		if (!temp->reference && !temp->dirty){
			temp2 = temp;
		}
		temp = temp->next;
		if (temp==prev->next) break;
	}
	if (temp2 != NULL){
		//use this entry
		if (DEBUG) printf("Pagefault - CASE 1  pid= %d page= %d\n", pid, page);
		computeCounts(temp->dirty);
		temp2->pid = pid;
		temp2->page= page;
		temp2->reference = 0;
		if (write) temp2->dirty = 1;
		else temp2->dirty = 0;
		resetReferencePrev(temp2);
		return 1;
	}

	//attempt to find entry with reference bit = 0, dirty bit = 1
	temp = prev->next;
	while (1){
		if (!temp->reference && temp->dirty){
			temp2 = temp;
		}
		temp = temp->next;
		if (temp==prev->next) break;
	}
	if (temp2 != NULL){
		//use this entry
		if (DEBUG) printf("Pagefault - CASE 2  pid= %d page= %d\n", pid, page);
		computeCounts(temp->dirty);
		temp2->pid = pid;
		temp2->page= page;
		temp2->reference = 0;
		if (write) temp2->dirty = 1;
		else temp2->dirty = 0;
		resetReferencePrev(temp2);
		return 1;
	}


	//attempt to find entry with reference bit = 1, dirty bit = 0
	temp = prev->next;
	while (1){
		if (temp->reference && !temp->dirty){
			temp2 = temp;
		}
		temp = temp->next;
		if (temp==prev->next) break;
	}
	if (temp2 != NULL){
		//use this entry
		if (DEBUG) printf("Pagefault - CASE 3  pid= %d page= %d\n", pid, page);
		computeCounts(temp->dirty);
		temp2->pid = pid;
		temp2->page= page;
		temp2->reference = 0;
		if (write) temp2->dirty = 1;
		else temp2->dirty = 0;
		resetReferencePrev(temp2);
		return 1;
	}

	//attempt to find entry with reference bit = 1, dirty bit = 1
	temp = prev->next;
	while (1){
		if (temp->reference && temp->dirty){
			temp2 = temp;
		}
		temp = temp->next;
		if (temp==prev->next) break;
	}
	if (temp2 != NULL){
		//use this entry
		if (DEBUG) printf("Pagefault - CASE 4  pid= %d page= %d\n", pid, page);
		computeCounts(temp->dirty);
		temp2->pid = pid;
		temp2->page= page;
		temp2->reference = 0;
		if (write) temp2->dirty = 1;
		else temp2->dirty = 0;
		resetReferencePrev(temp2);
		return 1;
	}

	return 0; // this should never happen
}

/* Remove()
 *
 * This function takes in a pid and removes all frames that correspond to that pid.
 */
void Remove(int pid) {
	struct page_entry *temp;

	temp = prev->next;
	while (temp != prev){
		if (temp->pid == pid){
			fill_count--;
			temp->pid = 0;
			temp->page = DEFAULT_PAGE;
			temp->reference = 0;
			temp->dirty = 0;
		}
		temp = temp->next;
	}
}

/* Access()
 *
 * This function takes in a pid, address, and write option bit. It calls find() in
 * an attempt to process that request for the process with that pid to access the
 * page corresponding to that address, then returns the return value of that call to
 * find().
 */
int Access(int pid, int address, int write) {
	if (find(pid, address, write)) return 1;
	else return 0;
}

/* Terminate()
 *
 * This function takes in a pid, makes a call to Remove(), then decrements the pid count
 * to indicate that there's room for a new process in the inverted page table.
 */
void Terminate(int pid) {
	if (DEBUG) printf("pid %d terminated\n", pid);
	// make room for a new process
	procCount--;
	Remove(pid);
}

int main(int argc, char* argv[]) {
	int i;
	struct page_entry *temp, *temp2;

	if (argc != 2 || atoi(argv[1]) < 0) {
        	printf("Usage: ./pagerep_esc <numer_of_rounds>\n");
        	return 0;
	}

	// initialize the page table (circular queue of pages)
	prev = (struct page_entry*) malloc(sizeof(struct page_entry));
	prev->pid = 0;
	prev->page = DEFAULT_PAGE;
	temp = (struct page_entry*) malloc(sizeof(struct page_entry));
	temp->pid = 0;
	temp->page = DEFAULT_PAGE;
	prev->next = temp;
	for (i = 0; i < TABLESIZE-2; i++){
		temp2 = (struct page_entry*) malloc(sizeof(struct page_entry));
		temp2->pid = 0;
		temp2->page = DEFAULT_PAGE;
		temp->next = temp2;
		temp = temp2;
	}
	temp->next = prev;

	// run simulation
	printf("MMU simulation started\n");
	Simulate(atoi(argv[1]));
	printf("MMU simulation completed\n");

	// de-initialize the page table
	for (i = 0; i < TABLESIZE-1; i++){
		prev = temp;
		temp = prev->next;
		free(prev);
	}
	free(temp);

	// output final results
	printf("\nTable Entries Filled: %d\n\nREADS: %d\nWRITE + READS: %d\nPAGEFAULTS: %d\n\n", fill_count,
			r_count, rw_count, r_count+rw_count);
}
