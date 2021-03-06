/***************************************************************************
 File:   pager.cc
 Author: Adela Yang and Son Ngo and Venecia Xu
 Date:   Dec 2015

 Description: 
			This program implements an external pager to handle virtual 
			memory requests from application processes. It will manage a 
			stimulated physical memory and a stimulated disk for all 
			application processes.
 
******************************************************************************/

/*****************************************************************************/
/* include files */
#include <cstdlib>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <queue>
#include "vm_pager.h"

/***************************************************************************/
/* constants */
static const int FAILURE = -1;
static const int SUCCESS = 0;
static const int NO_VALUE = -1;
static const unsigned long INVALID = 0x50000000;
static const unsigned long VM_ARENA_MAXADDR = (unsigned long)VM_ARENA_BASEADDR + VM_ARENA_SIZE;
static const unsigned int VM_ARENA_MAXPAGE = VM_ARENA_MAXADDR / VM_PAGESIZE;
static const unsigned int PAGE_TABLE_SIZE = VM_ARENA_MAXPAGE - VM_ARENA_BASEPAGE;
static const unsigned int RESIDENT_FLAG = 1;
static const unsigned int ASSOCIATION_FLAG = 2;
static const unsigned int CLOCK_NOT_EVICTED_FLAG = 3;
static const unsigned int CLOCK_EVICTED_FLAG = 4;

using namespace std;

/***************************************************************************/
/* structs */

//contains information of a resident virtual page
typedef struct {
	page_table_entry_t* pageTableEntryP;	//points to the entry in the corresponding page table
	unsigned long vPage;					//virtual page
	pid_t pid; 								//which process this node belongs to
	unsigned int modBit :1;
	unsigned int refBit :1;
} node;

//contains page table and lowest valid arena address information of a specfic process
typedef struct {
	page_table_t* pageTableP;
	unsigned long int lowestValidAddr; 
} process;

//contains extra information of any virtual page
typedef struct {
	unsigned int diskBlock;
	bool resident;
	unsigned int zeroFilledbit :1;
} vpageinfo;

/***************************************************************************/
/* globals variables */
static pid_t CurrentPid = 0;

static unsigned int PhysicalMemSize;			
static unsigned int DiskSize;					

map <unsigned int, node*> PhysMemMap;
map <pid_t, map<unsigned int, vpageinfo* > > AllPagesMap;
map <unsigned int, vpageinfo*>* CurrMapP;
map <pid_t, process> ProcessMap;		

queue<node*> ClockQueue;
queue<unsigned int> FreePhysMem;		
queue<unsigned int> FreeDiskBlocks;		

/***************************************************************************/
/* utility functions */
void updateInfo(node* tempNode, page_table_entry_t* tempEntry, 
				unsigned int vpage, unsigned int ppage, 
				unsigned int flag, bool write_flag);
int nextAvailablePhysMem();
int nextAvailableDiskBlock();
bool resident(unsigned int vpage);
void zeroFill (unsigned int ppage);

/***************************************************************************
 Function: 	vm_init
 Inputs:	number of pages in physical mem, number of disk blocks
			available on disk	
 Returns:	nothing
 Description: 
 		   	sets up data structures needed to begin accepting subsequent
 		    requests from processes
 ***************************************************************************/
void vm_init(unsigned int memory_pages, unsigned int disk_blocks){

	//allocate memory to page_table_base_register
	page_table_base_register = new (nothrow) page_table_t;
	if (page_table_base_register == NULL){
		exit(FAILURE);
	}

	//saving to global variables
	PhysicalMemSize = memory_pages;
	DiskSize = disk_blocks;

	//initializing free physical memory list
	for (unsigned int i = 0; i < PhysicalMemSize; i++){
		FreePhysMem.push(i);
	}

	//initializing free disk blocks list
	for (unsigned int i = 0; i < DiskSize; i++){
		FreeDiskBlocks.push(i);
	}
}

/***************************************************************************
 Function: 	vm_create
 Inputs:   	process id
 Returns:  	nothing
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

	//initializing everything in the page table by setting the read & write 
	//bit to 0 and 1
	for (unsigned int i = 0; i < PAGE_TABLE_SIZE; i++){
		pointer->ptes[i].read_enable = 0; 
		pointer->ptes[i].write_enable = 0; 
	}

	process proInfo;
	proInfo.lowestValidAddr = INVALID;
	proInfo.pageTableP = pointer;
	ProcessMap.insert(pair<pid_t, process> (pid, proInfo));
}

/***************************************************************************
 Function: 	vm_switch
 Inputs:   	process id
 Returns:  	nothing
 Description:
	       	OS scheduler runs a new process
	       	allows pager to do bookkeeping to register the fact that a new 
	        process is running
 ***************************************************************************/
void vm_switch(pid_t pid){
    
	// update the currentPid to the pid of the switched-to process
	CurrentPid = pid;

	//pointing the current pointer to the page table of switched-to process
	page_table_base_register = ProcessMap[CurrentPid].pageTableP;

	//point to the current map of virtual pages of the current process
	CurrMapP = &(AllPagesMap[CurrentPid]);	
}

/***************************************************************************
 Function: 	vm_fault
 Inputs:   	the address of the message and the write flag
 Returns:  	0 if successful handling fault, -1 if address to an invalid page
 Description:
	       	response to read or write fault from application
	       	determine which access in the arena will generate faults by setting
	       		read or write enable fields in page table
	       	determine which physical page is associated with a virtual page by 
	       		setting the ppage field in page table
	       	physical page should be associated with at most one virtual page
	       		in one process
 ***************************************************************************/
int vm_fault(void *addr, bool write_flag){

	unsigned long virtualAddr = (intptr_t) addr;
    
    //make sure there is enough space in the arena
	if (virtualAddr >= ProcessMap[CurrentPid].lowestValidAddr){
		return FAILURE;
	}

	//calculating page number associated with virtual address
	unsigned int pageNumber = virtualAddr / VM_PAGESIZE;
	page_table_entry_t* tempEntry = 
			&(page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE]);

	int nextPhysMem = 0;
	//if the page is not in resident
	if (!resident(pageNumber)){
		nextPhysMem = nextAvailablePhysMem();

		//there is no free physical memory, so activate the second-chance clock
		//algorithm to evict active pages
		if (nextPhysMem == NO_VALUE){
			while (true){
				node* curr = ClockQueue.front();
				ClockQueue.pop();	

				//if the examined page has not been accessed then evict
				if (curr->refBit == 0){
					int evictedPage = curr->pageTableEntryP->ppage;

					//evicting the currently examined page so set both 
					//read and write to 0
					curr->pageTableEntryP->read_enable = 0;
					curr->pageTableEntryP->write_enable = 0;

					//pointer to the evicted page in AllPagesMap
					vpageinfo* evictedP = (AllPagesMap[curr->pid])[curr->vPage];

					//updating residency of evicted page to false
					evictedP->resident = false;

					//write to swap space if the evicted page has been modified				
					if (evictedP->zeroFilledbit == 0){
						if (curr->modBit == 1){
							disk_write(evictedP->diskBlock, evictedPage);
							evictedP->zeroFilledbit = 1;	
						}
					}
					//only write if it is currently modified
					else if (curr->modBit == 1){
					 	disk_write(evictedP->diskBlock, evictedPage);
					}

					zeroFill(evictedPage);
					
					//read from disk only if the faulted page has ever been modified
					if ((*CurrMapP)[pageNumber]->zeroFilledbit == 1){
						disk_read((*CurrMapP)[pageNumber]->diskBlock, evictedPage);
					}
				
					//creating a new entry for the queue for replacement	
					node* tempNode = new (nothrow) node;

					updateInfo(tempNode, tempEntry, 
							   pageNumber, evictedPage, 
							   CLOCK_EVICTED_FLAG, write_flag);
					
					ClockQueue.push(tempNode);

					break;
				}
				else {
					updateInfo(curr, NULL, 0, 0, 
							   CLOCK_NOT_EVICTED_FLAG, write_flag);

					ClockQueue.push(curr);
				}
			}
		}
		//there is unused physical memory, then just associate the page with that memory
		else {
			//create a node in the memory
			node* tempNode = new (nothrow) node;

			updateInfo(tempNode, tempEntry, pageNumber, nextPhysMem, ASSOCIATION_FLAG, write_flag);

			ClockQueue.push(tempNode);

		}
	}
	//the page is resident, update the bits, change the protections
	else {
		unsigned int tempPhysPage = tempEntry->ppage;
		node* tempNode = PhysMemMap[tempPhysPage];		

		updateInfo(tempNode, tempEntry, 0, 0, RESIDENT_FLAG, write_flag);
	}

	return SUCCESS;
}

/***************************************************************************
 Function: 	vm_destroy
 Inputs:   	none
 Returns:  	nothing
 Description:
		   	current process exits
 		   	deallocate all resources held by the current process 
 	        (page table, physical pages, disk blocks, etc.)
 	        physical pages and disk blocks released should be put back 
 	        	on the corresponding free list
 ***************************************************************************/
void vm_destroy(){

	node* toDelete = new (nothrow) node;
	unsigned int queueSize = ClockQueue.size();

	//removing from ClockQueue
	for (unsigned int i = 0; i < queueSize; i++){
		toDelete = ClockQueue.front();
		ClockQueue.pop();

		if (toDelete->pid != CurrentPid){
			ClockQueue.push(toDelete);
		}
	}

	map<unsigned int, node*>::iterator it = PhysMemMap.begin();
	vpageinfo* vPageP = new (nothrow) vpageinfo;
	unsigned int ppageDelete;
	unsigned int vpageDelete;

	//free memories for entries in PhysMemMap and push free blocks that 
	//are associated with virtual pages being destroyed
	for (it = PhysMemMap.begin(); it != PhysMemMap.end(); ){
		toDelete = it->second;
		if (toDelete->pid == CurrentPid){
			vpageDelete = toDelete->vPage;
			vPageP = (*CurrMapP)[vpageDelete];
			ppageDelete = toDelete->pageTableEntryP->ppage;

			FreeDiskBlocks.push(vPageP->diskBlock);
			FreePhysMem.push(ppageDelete);

			(*CurrMapP).erase(vpageDelete);
			delete toDelete;
			PhysMemMap.erase(it++);
		}
		else {
			++it;
		}
	}

	delete vPageP;

	//free the rest of the blocks associated with non-resident virtual 
	//pages by going through the AllPagesMap
	for (map<unsigned int, vpageinfo*  >::iterator mapIt = (*CurrMapP).begin(); 
			mapIt != (*CurrMapP).end(); ){
		FreeDiskBlocks.push((mapIt->second)->diskBlock);
		delete mapIt->second;
		(*CurrMapP).erase(mapIt++);
	}

	//delete entries in ProcessMap
	delete ProcessMap[CurrentPid].pageTableP; 
	ProcessMap.erase(CurrentPid);

	page_table_base_register = NULL;
	CurrMapP = NULL;
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

	int diskBlock = nextAvailableDiskBlock();

	//not enough disk blocks
	if (diskBlock == NO_VALUE){ 
		return NULL;
	}

	unsigned long nextLowest = ProcessMap[CurrentPid].lowestValidAddr;

	//starting at base
	if (nextLowest == INVALID){
		nextLowest = (unsigned long)VM_ARENA_BASEADDR;
	}
	//not enough arena size
	else if (nextLowest == (unsigned long)VM_ARENA_BASEADDR + VM_ARENA_SIZE){ 
		return NULL;
	}

	//getting the page that corresponds to the valid address
	unsigned int validPageNum = nextLowest / VM_PAGESIZE;

	//after that, update the next highest valid bit
	ProcessMap[CurrentPid].lowestValidAddr = nextLowest + VM_PAGESIZE;

	//assign a specific disk block to any page number, and this will ALWAYS 
	//stay constant
	vpageinfo* infoP = new (nothrow) vpageinfo;
	infoP->diskBlock = diskBlock;
	infoP->zeroFilledbit = 0;
	infoP->resident = false;
	AllPagesMap[CurrentPid][validPageNum] = infoP;

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

	//check for out of bound conditions
	if (currAddr + len > ProcessMap[CurrentPid].lowestValidAddr || len <= 0 
		|| !((currAddr >= (unsigned long)VM_ARENA_BASEADDR) && (currAddr <= VM_ARENA_MAXADDR))){
		return FAILURE;
	}

	string s;
	unsigned int vpageNumber = 0;
	unsigned int pageOffSet = 0;
	unsigned long pAddr = 0;
	page_table_entry_t* tempEntry = new (nothrow) page_table_entry_t;

	for (unsigned long i = 0; i < len; i++){

		//translating from virtual address to physical address
		vpageNumber = currAddr / VM_PAGESIZE;
		tempEntry = &(page_table_base_register->ptes[vpageNumber - VM_ARENA_BASEPAGE]);

		//fault if needed
		if (!resident(vpageNumber) || tempEntry->read_enable == 0){
			if (vm_fault((void*)currAddr, false) == FAILURE){
				return FAILURE;
			}
		}

		pageOffSet = currAddr % VM_PAGESIZE;
		pAddr = tempEntry->ppage * VM_PAGESIZE;

		s.append(1, ((char*)pm_physmem)[pAddr + pageOffSet]);

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
 Function: updateInfo
 Inputs:   6 inputs
 		   pointer to node struct
 		   pointer to page_table_entry_t
 		   vpage Number
 		   ppage Number
 		   flag for action
 		   write flag
 Returns:  nothing
 Description:
	       updates the info of a page (the bits & ppage)
 ***************************************************************************/
void updateInfo(node* tempNode, page_table_entry_t* tempEntry, 
				unsigned int vpage, unsigned int ppage, 
				unsigned int flag, bool write_flag){

	if (flag == RESIDENT_FLAG){
		tempEntry->read_enable = 1;
		if (write_flag == true || tempNode->modBit == 1){
			tempEntry->write_enable = 1;
			tempNode->modBit = 1;
			(*CurrMapP)[tempNode->vPage]->zeroFilledbit = 1;
		}
		tempNode->refBit = 1;
	}
	else if (flag == ASSOCIATION_FLAG){
		//create a node in the memory
		tempEntry->ppage = ppage;
		tempEntry->read_enable = 1;
		tempEntry->write_enable = 0;
		tempNode->refBit = 1;
		tempNode->modBit = 0;

		//page has been modified because of write_flag
		if (write_flag == true){
			tempEntry->write_enable = 1;
			tempNode->modBit = 1;
			(*CurrMapP)[vpage]->zeroFilledbit = 1;
		}

		//update information
		tempNode->vPage = vpage;
		tempNode->pid = CurrentPid;
		tempNode->pageTableEntryP = tempEntry;

		//zero fill because trying to read 
		zeroFill(ppage);

		//update to true for page in memory
		(*CurrMapP)[vpage]->resident = true;
		PhysMemMap[ppage] = tempNode;
	}
	else if (flag == CLOCK_NOT_EVICTED_FLAG){
		tempNode->refBit = 0;
		tempNode->pageTableEntryP->read_enable = 0;
		tempNode->pageTableEntryP->write_enable = 0;
	}
	else if (flag == CLOCK_EVICTED_FLAG){
		tempEntry->ppage = ppage;
		tempNode->pageTableEntryP = tempEntry;
		tempNode->pid = CurrentPid;
		tempNode->vPage = vpage;					
		tempNode->refBit = 1;
		tempEntry->read_enable = 1;

		//update information based on write_flag
		if (write_flag == true){
			tempEntry->write_enable = 1;
			tempNode->modBit = 1;
			(*CurrMapP)[vpage]->zeroFilledbit = 1;
		}
		else {
			tempEntry->write_enable = 0;
			tempNode->modBit = 0;
		}

		//update residency of page being put to memory to true
		(*CurrMapP)[vpage]->resident = true;
		PhysMemMap[ppage] = tempNode;
	}
}

/***************************************************************************
 Function: nextAvailablePhysMem
 Inputs:   none
 Returns:  the next available physical memory (in terms of integers), o/w -1
 Description:
	       retrieves the next available physical memory and removes that
	       memory from the free memory vector
 ***************************************************************************/
int nextAvailablePhysMem(){

	if (FreePhysMem.empty()){
		return NO_VALUE;
	}
	else {
		int availPhysMem = FreePhysMem.front();
		FreePhysMem.pop();
		return availPhysMem;
	}
}

/***************************************************************************
 Function: nextAvailableDiskBlock
 Inputs:   none
 Returns:  the next available disk block to be written to (in terms of interger), o/w -1
 Description:
	       retrieves the next available disk block to write to
 ***************************************************************************/
int nextAvailableDiskBlock(){

	if (FreeDiskBlocks.empty()){
		return NO_VALUE;
	}
	else {
		int availDisk = FreeDiskBlocks.front();
		FreeDiskBlocks.pop();
		return availDisk;
	}
}

/***************************************************************************
 Function: 	resident
 Inputs:   	virtual page
 Returns:  	true if the page is resident
 Description:
	       check to see if the virutal page is resident
 ***************************************************************************/
bool resident(unsigned int vPage){

	return (AllPagesMap[CurrentPid])[vPage]->resident;
}

/***************************************************************************
 Function: 	zeroFill
 Inputs:  	physical page number
 Returns:  	nothing
 Description:
 			zero fill memory associated with the given physical page number
 ***************************************************************************/
void zeroFill(unsigned int ppage){

	for (unsigned int i = 0; i < VM_PAGESIZE; i++){
		((char*)pm_physmem)[ppage * VM_PAGESIZE + i] = 0;
	}
}

/****** END OF FILE ********************************************************/