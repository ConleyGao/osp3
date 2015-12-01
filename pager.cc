/****************************************************************************
 File:   pager.cc
 Author: Adela Yang and Son Ngo and Venecia Xu
 Date:   Nov 2015

 Description:

 Notes: 
 Run this program with argument on the command line

 Running instructions:

	Compile the pager first with the command line: 

		g++ -Wall -o pager pager.cc libvm_pager.a -ldl

	Then open a new terminal window, and compile with the command line: 

	 	g++ -Wall -o test test.<memorypages>.cc libvm_app.a -ldl 

	Then on that same terminal window (of test case), run ./test

	See results and happy debugging...
 	
 Data Structures:

   
******************************************************************************/

/*****************************************************************************/
/* include files */
#include <cstdlib>
#include <stdint.h>
#include <iostream>
#include "vm_pager.h"
#include <map>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

/***************************************************************************/
/* constants */
static const int FAILURE = -1;
static const int SUCCESS = 0;
static const unsigned long INVALID = 0x50000000;
static const int NO_VALUE = -1;

using namespace std;

/***************************************************************************/
/* structs */
typedef struct {
	page_table_entry_t* pageTableEntryP;	//points to the entry in the corresponding page table
	unsigned long vPage;					//virtual page
	pid_t pid; 								//which process this node belongs to
	int diskBlock;
	unsigned int modBit;
	unsigned int refBit;
	//node *next;
} node;

/***************************************************************************/
/* globals variables */
static pid_t CurrentPid = 0; 					//pid of currently running process
static unsigned int PhysicalMemSize;			//size of physical memory
static unsigned int DiskSize;
static bool* PhysMemP;
static bool* DiskBlocksP;
map <pid_t, unsigned long int> AppArenaMap;		//the value is the highest valid virtual address
map <pid_t, page_table_t*> PTMap;				//page table map
map <unsigned int, node*> PhysMemMap;

vector<unsigned int> FreePhysMemList; 		//store the physical pages (number) that are free

queue<node*> ClockQueue;

/***************************************************************************/
/* prototypes */
int nextAvailablePhysMem();
int nextAvailableDiskBlock();
bool resident(unsigned int pageNumber);
bool onDisk(unsigned long addr);
unsigned long pageTranslate(unsigned long vPage);

/***************************************************************************
 Function: vm_init
 Inputs:   number of pages in physical mem, number of disk blocks
			available on disk	
 Returns:  nothing
 Description: 
 		   sets up data structures needed to begin accepting subsequent
 		   	requests from processes
 ***************************************************************************/
void vm_init(unsigned int memory_pages, unsigned int disk_blocks){

	// head = new (nothrow) node;
	// if (head == NULL){
	// 	exit(FAILURE);
	// }
	// head->next = NULL;
	// tail = new (nothrow) node;
	// if (tail == NULL){
	// 	exit(FAILURE);
	// }
	// tail->next = NULL;

	page_table_base_register = new (nothrow) page_table_t;
	if (page_table_base_register == NULL){
		exit(FAILURE);
	}

	//saving to global variables
	PhysicalMemSize = memory_pages;
	DiskSize = disk_blocks;

	//initializing array of phys mem
	PhysMemP = new (nothrow) bool [PhysicalMemSize];
	if (PhysMemP == NULL){
		exit(FAILURE);
	}
	for (unsigned int i = 0; i < PhysicalMemSize; i++){
		PhysMemP[i] = false;
	}

	//initializing array of disk blocks
	DiskBlocksP = new (nothrow) bool [DiskSize];
	if (DiskBlocksP == NULL){
		exit(FAILURE);
	}
	for (unsigned int i = 0; i < DiskSize; i++){
		DiskBlocksP[i] = false;
	}

	//initializing free list (unused)
	for (unsigned int i = 0; i < PhysicalMemSize; i++){
		FreePhysMemList.push_back(i);
	}

	//initialize entry of disk block array to be invalid
	// for (int i = 0; i < DiskSize; i++){
	// 	DiskBlocksP[i] = INVALID;
	// }
}

/***************************************************************************
 Function: vm_create
 Inputs:   process id
 Returns:  nothing
 Description:
		   new application starts
		   initialize data structures to handle process
		   initial page tables should be empty, new process will 
		    not be running until after it is switched to
 ***************************************************************************/
void vm_create(pid_t pid){

	//creating a page table for the process
	page_table_t *pointer = new (nothrow) page_table_t;
	if (pointer == NULL){
		exit(1);
	}

	//creating a new entry in PTMap
	PTMap.insert(pair<pid_t, page_table_t*> (pid, pointer));

	//creating a new entry in AppArenaMap
	AppArenaMap.insert(pair<pid_t, unsigned long int> (pid, INVALID));

}

/***************************************************************************
 Function: vm_switch
 Inputs:   process id
 Returns:  nothing
 Description:
	       OS scheduler runs a new process
	       allows pager to do bookkeeping to register the fact that a new 
	        process is running
 ***************************************************************************/
void vm_switch(pid_t pid){
	cout << "switching to: " << pid << endl;
	// update the currentPid to the pid of the switched-to process
	CurrentPid = pid;

	//pointing the current pointer to the page table of switched-to process
	page_table_base_register = PTMap[CurrentPid];
}

/***************************************************************************
 Function: vm_fault
 Inputs:   
 Returns:  0 if successful handling fault, -1 if address to an invalid page
 Description:
	       response to read or write fault by application
	       determine which accesss i the arena will generate faults by setting
	       	read or write enable fields in page table
	       determine which physical page is associated with a virtual page by 
	       	setting the ppage field in page table
	       physical page should be associated with at most one virtual page
	       	in one process
 ***************************************************************************/
int vm_fault(void *addr, bool write_flag){

	unsigned long virtualAddr = (intptr_t) addr;
	//cout << "virtual address: " << virtualAddr << endl;
	// cout << "app arena current pid: " << hex << AppArenaMap[currentPid] << endl;
	//faulting on an invalid address
	if (virtualAddr >= AppArenaMap[CurrentPid]) {
		return FAILURE; //-1
	}

	//calculating page number associated with virtual address
	unsigned long pageNumber = virtualAddr / VM_PAGESIZE;
	cout << "virtual page number: " << hex << pageNumber << endl;
	int pageOffSet = virtualAddr % VM_PAGESIZE;
	cout << "virtual page offset: " << pageOffSet << endl;
	page_table_entry_t* tempEntry = &(page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE]);
	//cout << "what I am looking at " << tempEntry << endl;
	cout << "physical page number: " << hex << tempEntry->ppage << endl;
	unsigned int ppageNumber = tempEntry->ppage;

	int nextPhysMem = 0;
	//if the page is not in resident (i.e not in physical memory)
	if (!resident(ppageNumber)){
		cout << "not resident" << endl;
		nextPhysMem = nextAvailablePhysMem();
		cout << "next availabe physical memory is: " << nextPhysMem << endl;
		//there is no free physical memory, so have to run second-chance clock 
		//algorithm to evict active pages
		if (nextPhysMem == NO_VALUE){
			node* curr = ClockQueue.front();
			ClockQueue.pop();
			while (true){
				if (curr->refBit == 0){
					int availPhysPage = curr->pageTableEntryP->ppage;
					cout << "evicting physical page: " << availPhysPage << endl;

					//replacing that page with the current page
					tempEntry->ppage = availPhysPage;

					int nextBlock = nextAvailableDiskBlock();
					if (nextBlock == NO_VALUE){
						return FAILURE;
					}

					//need to also clear data in physical memory (pm_physmem) when
					//evicting a page

					/* 

					Question: what happen if we are trying to access a page 
					that is not in memory, but ON THE DISK. Should we always check for that?
					How do we know if a page is on the disk when it's not in memory?

					*/

					node* newNode = new (nothrow) node;

					//write to swap space if page has been modified
					if (curr->modBit == 1){
						disk_write(nextBlock, curr->pageTableEntryP->ppage);
						newNode->diskBlock = nextBlock;
						DiskBlocksP[nextBlock] = true;
					}
					else {
						newNode->diskBlock = NO_VALUE;
					}

					//creating a new entry for the queue for replacement
					
					newNode->pageTableEntryP = tempEntry;
					newNode->modBit = 0;
					//refBit should be 1 because accessed
					newNode->refBit = 1;
					newNode->pid = CurrentPid;
					newNode->vPage = pageNumber;

					//(!)this part I don't know if it's correct or not
					if (write_flag == true){
						tempEntry->write_enable = 1;
						newNode->modBit = 1;
					}
					else {
						tempEntry->write_enable = 0;
						newNode->modBit = 0;
					}
					tempEntry->read_enable = 1;
					//(!)that part ends here

					//updating queue and physical memory map
					ClockQueue.push(newNode);
					PhysMemP[availPhysPage] = true;
					PhysMemMap[availPhysPage] = newNode;

					break;
				}
				else {
					curr->refBit = 0;
					ClockQueue.push(curr);
				}
			}
		}
		//there is unused physical memory, then just associate the page with that memory
		else {
			nextPhysMem = (unsigned int) nextPhysMem;
			cout << "there is extra memory" << endl;
			//cout << "and here is what I am also looking at " << tempEntry << endl;
			tempEntry->ppage = nextPhysMem;
			cout << "assigned physical page: " << tempEntry->ppage << endl;
			//tempEntry->read_enable = 1;
			if (write_flag == true){
				tempEntry->write_enable = 1;
			}
			else {
				tempEntry->read_enable = 1;
			}

			//cout << "what I need to look at " << &(page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE]) << endl;
			// cout << "node write " 
			// 	 << page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE].write_enable << endl;
			// cout << "node read " 
			// 	 << page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE].read_enable << endl;
			// cout << "node ppage " 
			//  	 << page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE].ppage << endl;
			
			//create a node in the memory
			node* nodeCreate = new (nothrow) node;
			nodeCreate->modBit = 0;
			if (write_flag == true){
				nodeCreate->modBit = 1;
			}
			nodeCreate->refBit = 1;
			nodeCreate->vPage = pageNumber;
			nodeCreate->pid = CurrentPid;
			nodeCreate->diskBlock = NO_VALUE;
			nodeCreate->pageTableEntryP = tempEntry;

			//stick the node to the end of clock queue
			ClockQueue.push(nodeCreate);
			//cout << "size of clock queue: " << ClockQueue.size() << endl;

			//zero-filling for association (confused)
			for (unsigned int i = nextPhysMem; i < (unsigned int)nextPhysMem + VM_PAGESIZE; i++){
				((char*)pm_physmem)[i] = '\0';
			}

			// for (unsigned int i = nextPhysMem * VM_PAGESIZE; i < VM_PAGESIZE; i++){
			// 	cout << "phys at " << i << ": " << ((char*)pm_physmem)[i] << endl;
			// }

			PhysMemP[nextPhysMem] = true;
			PhysMemMap[nextPhysMem] = nodeCreate;
		}
	}
	//the page is resident, update the bits, change the protections
	else {
		unsigned int tempPhysPage = tempEntry->ppage/VM_PAGESIZE;
		node* tempNode = PhysMemMap[tempPhysPage];
		
		if (write_flag == true){
			tempEntry->write_enable = 1;
			tempNode->modBit = 1;
		}
		else {
			tempEntry->read_enable = 1;
		}
		tempNode->refBit = 1;
	}

	return SUCCESS;

}

/***************************************************************************
 Function: vm_destroy
 Inputs:   none
 Returns:  nothing
 Description:
		   current process exits
 		   deallocate all resources held by the current process 
 	        (page table, physical pages, disk blocks, etc.)
 	        physical pages released should be put back on the free list
 ***************************************************************************/
void vm_destroy(){
	cout << "destroying pid: " << CurrentPid << endl;
	//delete page_table_base_register;
	page_table_base_register = NULL;
	AppArenaMap.erase(CurrentPid);
	PTMap.erase(CurrentPid);
	map<unsigned int, node*>::iterator it = PhysMemMap.begin();

	//delete in physical memory
	node* toDelete = new (nothrow) node;
	unsigned long ppageDelete;
	for (it = PhysMemMap.begin(); it != PhysMemMap.end(); ){
		toDelete = it->second;
		if (toDelete->pid == CurrentPid){
			if (toDelete->diskBlock != NO_VALUE){
				DiskBlocksP[toDelete->diskBlock] = false;
			}
			ppageDelete = toDelete->pageTableEntryP->ppage;
			for (unsigned int i = 0; i < VM_PAGESIZE; i++){
				((char*)pm_physmem)[ppageDelete * VM_PAGESIZE + i] = '\0';
			}
			PhysMemP[ppageDelete] = false;
			PhysMemMap.erase(it++);
		}
		else {
			++it;
		}
	}

	//delete in disk block
	cout << "finished destroying " << CurrentPid << endl;
	cout << "============================================" << endl;
}

/***************************************************************************
 Function: vm_extend
 Inputs:   none
 Returns:  lowest-numbered byte of the new valid virtual page, o/w is NULL 
 Description:
		   process make another virtual page in its arena valid
		   ensure enough available disk blocks to hold all valid virtual pages
		   each byte of a newly extended virtual page initialized with value 0
 ***************************************************************************/
void * vm_extend(){
	//check if having enough disk blocks to write if necessary
	if (nextAvailableDiskBlock() == NO_VALUE){
		return NULL;
	}

	unsigned long int nextLowest = AppArenaMap[CurrentPid];

	//no valid address, starting at base
	if (nextLowest == INVALID){
		nextLowest = (unsigned long)VM_ARENA_BASEADDR;
	}
	//not enough arena size
	else if (nextLowest == (unsigned long)VM_ARENA_BASEADDR + VM_ARENA_SIZE){ 
		return NULL;
	}
	// //next lowest valid virtual address
	// else {
	// 	nextLowest++;
	// }

	//cout << "returning " << hex << nextLowest << endl;

	//getting the page that corresponds to the valid address
	unsigned int validPageNum = nextLowest / VM_PAGESIZE - VM_ARENA_BASEPAGE;

	//after that, update the next highest valid bit
	AppArenaMap[CurrentPid] = nextLowest + VM_PAGESIZE;

	//initializing the entry in page table
	//find the page table entry that corresponds to the lowest virtual address
	//set the read bit and write bit to 0 --> unused page
	//need pointer so we will change the value in the actual page table, not just the copy
	page_table_entry_t* tempEntry = &(page_table_base_register->ptes[validPageNum]);
	tempEntry->read_enable = 0;
	tempEntry->write_enable = 0;
	tempEntry->ppage = INVALID;

	return (void*)nextLowest;
}

/***************************************************************************
 Function: vm_syslog
 Inputs:   pointer to an array of bytes in virtual address space and 
 			length of the array
 Returns:  -1 if message no on a valid arena page or length is 0, o/w is 0
 Description:
		   check message is in valid pages of arena
		   copy array into string in pager's address space and print it
 ***************************************************************************/
int vm_syslog(void *message, unsigned int len){

	unsigned long currAddr = (intptr_t) message;

	if (currAddr + len > AppArenaMap[CurrentPid] || len == 0){
		return FAILURE;
	}

	string s;
	int vpageNumber = 0;
	int pageOffSet = 0;
	int pAddr = 0;

	for (unsigned long i = 0; i < len; i++){
		//translating from virtual address to physical address
		vpageNumber = currAddr / VM_PAGESIZE;
		pageOffSet = currAddr % VM_PAGESIZE;
		pAddr = page_table_base_register->ptes[vpageNumber - VM_ARENA_BASEPAGE].ppage * VM_PAGESIZE;

		//constructing the string
		s += string(1,((char*)pm_physmem)[pAddr + pageOffSet]);

		//going to the next bytes (address)
		++currAddr;
	}	

	cout << "syslog \t\t\t" << s << endl;

	return SUCCESS;

}

/***************************************************************************
	UTILITY FUNCTIONS
 ***************************************************************************/

/***************************************************************************
 Function: nextAvailablePhysMem
 Inputs:   none
 Returns:  the next available physical memory (in terms of integers)
 Description:
	
 ***************************************************************************/
int nextAvailablePhysMem(){
	for (unsigned int i = 0; i < PhysicalMemSize; i++){
		if (PhysMemP[i] == false){
			cout << "returning available: " << i << endl;
			return i;
		}
	}
	return NO_VALUE;
}

/***************************************************************************
 Function: nextAvailableDiskBlock
 Inputs:   none
 Returns:  the next available disk block to be written to (in terms of interger)
 Description:
	
 ***************************************************************************/
int nextAvailableDiskBlock(){
	for (unsigned int i = 0; i < DiskSize; i++){
		if (DiskBlocksP[i] == false){
			return i;
		}
	}
	return NO_VALUE;
}

/***************************************************************************
 Function: resdient
 Inputs:   none
 Returns:  true if the page is resident
 Description:
	
 ***************************************************************************/
bool resident(unsigned int ppageNumber){
	// if (PhysMemMap[ppageNumber] != NULL){
	// 	return true;
	// }
	// else {
	// 	return false;
	// }
	return !(ppageNumber == INVALID);
}

/***************************************************************************
 Function: onDisk
 Inputs:   none
 Returns:  true if the page is resident
 Description:
	
 ***************************************************************************/
bool onDisk(unsigned long address){
	for (unsigned int i = 0; i < DiskSize; i++){
		if (DiskBlocksP[i] == true){
			return true;
		}
	}
	return false;
}

/***************************************************************************
 Function: pageTranslate
 Inputs:   address
 Returns:  nothing
 Description:
	given an address can translate it into a page
 ***************************************************************************/
unsigned long pageTranslate(unsigned long vPage);






/****** END OF FILE *********************************************************/