/***************************************************************************
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
#include <stdio.h>
#include <iostream>
#include "vm_pager.h"
#include <map>
#include <set>
#include <string>
#include <string.h>
#include <vector>
#include <queue>
#include <algorithm>

/***************************************************************************/
/* constants */
static const int FAILURE = -1;
static const int SUCCESS = 0;
static const unsigned long INVALID = 0x50000000;
static const int NO_VALUE = -1;
static const unsigned int PAGE_TABLE_SIZE = ((unsigned long)VM_ARENA_BASEADDR + VM_ARENA_SIZE)/VM_PAGESIZE - VM_ARENA_BASEPAGE;
static const unsigned int RESIDENT_FLAG = 1;
static const unsigned int ASSOCIATION_FLAG = 2;
static const unsigned int CLOCK_NOT_EVICTED_FLAG = 3;
static const unsigned int CLOCK_EVICTED_FLAG = 4;

using namespace std;

/***************************************************************************/
/* structs */
typedef struct {
	page_table_entry_t* pageTableEntryP;	//points to the entry in the corresponding page table
	unsigned long vPage;					//virtual page
	pid_t pid; 								//which process this node belongs to
	unsigned int modBit :1;
	unsigned int refBit :1;
} node;

typedef struct {
	page_table_t* pageTableP;
	unsigned long int lowestValidAddr; 
} process;

typedef struct {
	unsigned int diskBlock;
	bool resident;
	unsigned int zeroFilledbit :1;
} vpageinfo;

/***************************************************************************/
/* globals variables */
static pid_t CurrentPid = 0; 					//pid of currently running process
static unsigned int PhysicalMemSize;			//size of physical memory
static unsigned int DiskSize;

//pair of data structurs for physical memory
map <unsigned int, node*> PhysMemMap;
map <pid_t, map<unsigned int, vpageinfo* > > DiskBlockMap;
map <unsigned int, vpageinfo*>* currMapP;

map <pid_t, process> ProcessMap;		//combining the above 2 maps

queue<node*> ClockQueue;
queue<unsigned int> FreePhysMem;
queue<unsigned int> FreeDiskBlocks;

/***************************************************************************/
/* prototypes */
void updateInfo(node* tempNode, page_table_entry_t* tempEntry, unsigned int vpage, unsigned int ppage, unsigned int flag, bool write_flag);
int nextAvailablePhysMem();
int nextAvailableDiskBlock();
bool resident(unsigned int vpage);
unsigned long pageTranslate(unsigned long vPage);
void zeroFill (unsigned int ppage);

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

	page_table_base_register = new (nothrow) page_table_t;
	if (page_table_base_register == NULL){
		exit(FAILURE);
	}

	//saving to global variables
	PhysicalMemSize = memory_pages;
	DiskSize = disk_blocks;
#ifdef PRINT
	cerr << "Number of disk blocks: " << DiskSize << endl;
#endif
	//initializing free physical memory list
	for (unsigned int i = 0; i < PhysicalMemSize; i++){
		// cerr << "pushing " << i << " to memory free" << endl;
		FreePhysMem.push(i);
	}

	//initializing free disk blocks list
	for (unsigned int i = 0; i < DiskSize; i++){
		FreeDiskBlocks.push(i);
	}

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
 Function: vm_switch
 Inputs:   process id
 Returns:  nothing
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
	currMapP = &(DiskBlockMap[CurrentPid]);
	
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
    
#ifdef PRINT
	cerr << "virtual address: " << virtualAddr << endl;
#endif

	if (virtualAddr >= ProcessMap[CurrentPid].lowestValidAddr){
		return FAILURE;
	}

	//calculating page number associated with virtual address
	unsigned int pageNumber = virtualAddr / VM_PAGESIZE;
	page_table_entry_t* tempEntry = &(page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE]);
    
	//create a pointer that points to faulted page entry in DiskBlockMap
	//map<unsigned int, vpageinfo*>* faultP = DiskBlockMap

#ifdef PRINT
    cerr << "virtual page number: " << hex << pageNumber << endl;
	cerr << "what I am looking at " << tempEntry << endl;
	cerr << "physical page number: " << hex << tempEntry->ppage << endl;
#endif

	int nextPhysMem = 0;
	//if the page is not in resident (i.e not in physical memory)
	if (!resident(pageNumber)){
		nextPhysMem = nextAvailablePhysMem();
        
#ifdef PRINT
        cerr << "not resident" << endl;
		cerr << "next availabe physical memory is: " << nextPhysMem << endl;
#endif
        
		//there is no free physical memory, so have to run second-chance clock
		//algorithm to evict active pages
		if (nextPhysMem == NO_VALUE){
			while (true){
				// cerr << "size of queue: " << ClockQueue.size() << endl;
				node* curr = ClockQueue.front();
				ClockQueue.pop();	
				if (curr->refBit == 0){
					int evictedPage = curr->pageTableEntryP->ppage;
					// cerr << "evicting physical page: " << evictedPage << endl;

					//evicting the page of curr so set both read and write to 0
					curr->pageTableEntryP->read_enable = 0;
					curr->pageTableEntryP->write_enable = 0;

					//updating residency of evicted page to false

					//pointer to the evicted page in DiskBlockMap
					vpageinfo* evictedP = (DiskBlockMap[curr->pid])[curr->vPage];

					evictedP->resident = false;

					//write to swap space if the evicted page has been modified				
					if (evictedP->zeroFilledbit == 0){
						if (curr->modBit == 1){
							disk_write(evictedP->diskBlock, evictedPage);
							evictedP->zeroFilledbit = 1;	
						}
					}
					else if (curr->modBit == 1){
					 	disk_write(evictedP->diskBlock, evictedPage);
					}

					zeroFill(evictedPage);
					
					// //the faulted page is already on disk, restore the information
					if ((*currMapP)[pageNumber]->zeroFilledbit == 1){
						disk_read((*currMapP)[pageNumber]->diskBlock, evictedPage);
					}
				
					cerr << "Page " << hex << curr->vPage << " of process " << curr->pid << " evicted" << endl;

					//creating a new entry for the queue for replacement	
					node* tempNode = new (nothrow) node;

					updateInfo(tempNode, tempEntry, pageNumber, evictedPage, CLOCK_EVICTED_FLAG, write_flag);

					break;
				}
				else {
					updateInfo(curr, NULL, 0, 0, CLOCK_NOT_EVICTED_FLAG, write_flag);
				}
			}
		}
		//there is unused physical memory, then just associate the page with that memory
		else {
#ifdef PRINT
			cerr << "there is extra memory" << endl;
			cerr << "assigned physical page: " << tempEntry->ppage << endl;
#endif
			//create a node in the memory
			node* tempNode = new (nothrow) node;

			updateInfo(tempNode, tempEntry, pageNumber, nextPhysMem, ASSOCIATION_FLAG, write_flag);

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
    
#ifdef PRINT
	cerr << "destroying pid: " << CurrentPid << endl;
#endif
    
	//delete page_table_base_register;
	page_table_base_register = NULL;

	ProcessMap.erase(CurrentPid);

	map<unsigned int, node*>::iterator it = PhysMemMap.begin();

	vpageinfo* vPageP = new (nothrow) vpageinfo;

	//remove from ClockQueue
	node* toDelete = new (nothrow) node;
	unsigned int queueSize = ClockQueue.size();

	for (unsigned int i = 0; i < queueSize; i++){
		toDelete = ClockQueue.front();
		ClockQueue.pop();
		if (toDelete->pid != CurrentPid){
			ClockQueue.push(toDelete);
		}
	}

	//delete in physical memory, push back physical pages to free memory list
	//push free blocks that are associated with virtual pages being destroyed
	//to reduce the traverse time in DiskBlockMap to free the rest of the blocks
	unsigned int ppageDelete;
	unsigned int vpageDelete;
	for (it = PhysMemMap.begin(); it != PhysMemMap.end(); ){
		toDelete = it->second;
		vpageDelete = toDelete->vPage;
		vPageP = (*currMapP)[vpageDelete];
		if (toDelete->pid == CurrentPid){
			ppageDelete = toDelete->pageTableEntryP->ppage;
			FreeDiskBlocks.push(vPageP->diskBlock);
			FreePhysMem.push(ppageDelete);
			(*currMapP).erase(vpageDelete);
			PhysMemMap.erase(it++);
		}
		else {
			++it;
		}
	}

	//free the rest of the blocks associated with non-resident virtual pages
	for (map<unsigned int, vpageinfo*  >::iterator mapIt = (*currMapP).begin(); mapIt != (*currMapP).end(); mapIt++){
		FreeDiskBlocks.push((mapIt->second)->diskBlock);
	}	

// #ifdef PRINT
	cerr << "size of disk queue is: " << FreeDiskBlocks.size() << endl;
	cerr << "size of memory queue is: " << FreePhysMem.size() << endl;

	//delete in disk block
	cerr << "finished destroying " << CurrentPid << endl;
	cerr << "============================================" << endl;
// #endif
    
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
	int diskBlock = nextAvailableDiskBlock();
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
    
#ifdef PRINT
	cerr << "returning " << hex << nextLowest << endl;
#endif

	//getting the page that corresponds to the valid address
	unsigned int validPageNum = nextLowest / VM_PAGESIZE;

	//after that, update the next highest valid bit
	ProcessMap[CurrentPid].lowestValidAddr = nextLowest + VM_PAGESIZE;

	//assign a specific disk block to any page number, and this will ALWAYS stay constant
#ifdef PRINT
    cerr << "assiging block " << diskBlock << " to " << validPageNum << " of " << CurrentPid << endl;
#endif
    
	vpageinfo* infoP = new (nothrow) vpageinfo;
	infoP->diskBlock = diskBlock;
	infoP->zeroFilledbit = 0;
	DiskBlockMap[CurrentPid][validPageNum] = infoP;

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
    
	if (currAddr + len > ProcessMap[CurrentPid].lowestValidAddr || len == 0){
		return FAILURE;
	}

	string s;
	unsigned int vpageNumber = 0;
	unsigned int pageOffSet = 0;
	page_table_entry_t* tempEntry = new page_table_entry_t;
	int pAddr = 0;

	for (unsigned long i = 0; i < len; i++){
		//translating from virtual address to physical address
		vpageNumber = currAddr / VM_PAGESIZE;
		tempEntry = &(page_table_base_register->ptes[vpageNumber - VM_ARENA_BASEPAGE]);
		if (!resident(vpageNumber)){
			// cerr << "i'm not resident" << endl;
			if (vm_fault((void*)currAddr, false) == FAILURE){
				return FAILURE;
			}
		}
		if (!resident(vpageNumber)){
			return FAILURE;
		}
		// cerr << "vpageNumber = " << vpageNumber << endl;
		pageOffSet = currAddr % VM_PAGESIZE;

		pAddr = tempEntry->ppage * VM_PAGESIZE;

		//constructing the string
		//s += string(1,((char*)pm_physmem)[pAddr + pageOffSet]);

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

void updateInfo(node* tempNode, page_table_entry_t* tempEntry, unsigned int vpage, unsigned int ppage, unsigned int flag, bool write_flag){
	if (flag == RESIDENT_FLAG){
		tempEntry->read_enable = 1;
		tempNode->refBit = 1;
		if (write_flag == true){
			tempEntry->write_enable = 1;
			tempNode->modBit = 1;
			(*currMapP)[tempNode->vPage]->zeroFilledbit = 1;
		}
	}
	else if (flag == ASSOCIATION_FLAG){
		//create a node in the memory
		tempEntry->ppage = ppage;
		tempEntry->read_enable = 1;
		tempEntry->write_enable = 0;
		tempNode->refBit = 1;
		tempNode->modBit = 0;
		if (write_flag == true){
			tempEntry->write_enable = 1;
			tempNode->modBit = 1;
			(*currMapP)[vpage]->zeroFilledbit = 1;
		}
		tempNode->vPage = vpage;
		tempNode->pid = CurrentPid;
		//tempNode->diskBlock = (*currMapP)[vpage]->diskBlock;
		tempNode->pageTableEntryP = tempEntry;

		//zero fill because trying to read (what abour writing?)
		zeroFill(ppage);

		//stick the node to the end of clock queue
		ClockQueue.push(tempNode);

		//update to true for page in memory
		(*currMapP)[vpage]->resident = true;

		// PhysMemP[nextPhysMem] = true;
		PhysMemMap[ppage] = tempNode;
	}
	else if (flag == CLOCK_NOT_EVICTED_FLAG){
		tempNode->refBit = 0;
		tempNode->pageTableEntryP->read_enable = 0;
		tempNode->pageTableEntryP->write_enable = 0;
		ClockQueue.push(tempNode);
	}
	else if (flag == CLOCK_EVICTED_FLAG){
		tempEntry->ppage = ppage;
		tempNode->pageTableEntryP = tempEntry;
		tempNode->pid = CurrentPid;
		tempNode->vPage = vpage;					
		tempNode->refBit = 1;

		if (write_flag == true){
			tempEntry->write_enable = 1;
			tempEntry->read_enable = 1;
			tempNode->modBit = 1;
			(*currMapP)[vpage]->zeroFilledbit = 1;
		}
		else {
			tempEntry->write_enable = 0;
			tempEntry->read_enable = 1;
			tempNode->modBit = 0;
		}

		//updating queue and physical memory map
		ClockQueue.push(tempNode);

		//update residency of page being put to memory to true
		(*currMapP)[vpage]->resident = true;

		// PhysMemP[evictedPage] = true;
		PhysMemMap[ppage] = tempNode;
	}
}

/***************************************************************************
 Function: nextAvailablePhysMem
 Inputs:   none
 Returns:  the next available physical memory (in terms of integers)
 Description:
	
 ***************************************************************************/
int nextAvailablePhysMem(){

	if (FreePhysMem.empty()){
		return NO_VALUE;
	}
	else {
		int avaiPhysMem = FreePhysMem.front();
		FreePhysMem.pop();
		// cerr << "size of free memory: " << FreePhysMem.size() << endl;
		return avaiPhysMem;
	}
}

/***************************************************************************
 Function: nextAvailableDiskBlock
 Inputs:   none
 Returns:  the next available disk block to be written to (in terms of interger)
 Description:
	
 ***************************************************************************/
int nextAvailableDiskBlock(){
	if (FreeDiskBlocks.empty()){
		return NO_VALUE;
	}
	else {
		int avaiDisk = FreeDiskBlocks.front();
		FreeDiskBlocks.pop();
		//cerr << "next block returning " << avaiDisk << endl;
		return avaiDisk;
	}
}

/***************************************************************************
 Function: resdient
 Inputs:   none
 Returns:  true if the page is resident
 Description:
	
 ***************************************************************************/
bool resident(unsigned int vPage){
	return (DiskBlockMap[CurrentPid])[vPage]->resident;
}

/***************************************************************************
 Function: zeroFill
 Inputs:   physical page number
 Returns:  nothing
 Description:
	zero fill all the data associated with the given physical page number
 ***************************************************************************/
void zeroFill(unsigned int ppage){
	for (unsigned int i = 0; i < VM_PAGESIZE; i++){
		((char*)pm_physmem)[ppage * VM_PAGESIZE + i] = 0;
	}
}

/****** END OF FILE ********************************************************/