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
static const int PAGE_TABLE_SIZE = ((unsigned long)VM_ARENA_BASEADDR + VM_ARENA_SIZE)/VM_PAGESIZE - VM_ARENA_BASEPAGE;

using namespace std;

/***************************************************************************/
/* structs */
typedef struct {
	page_table_entry_t* pageTableEntryP;	//points to the entry in the corresponding page table
	unsigned long vPage;					//virtual page
	pid_t pid; 								//which process this node belongs to
	int diskBlock;
	// unsigned int zeroFilledbit : 1;			//don't know why we need this
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
	//unsigned int zeroFilledbit :1;
} vpageinfo;

/***************************************************************************/
/* globals variables */
static pid_t CurrentPid = 0; 					//pid of currently running process
static unsigned int PhysicalMemSize;			//size of physical memory
static unsigned int DiskSize;

//pair of data structurs for physical memory
// static bool* PhysMemP;
map <unsigned int, node*> PhysMemMap;
//pair of data structures for disk blocks
//static bool* DiskBlocksP;
map <unsigned int, map<pid_t, vpageinfo> > DiskBlockMap;

map <pid_t, unsigned long int> AppArenaMap;		//the value is the highest valid virtual address
map <pid_t, page_table_t*> PTMap;				//page table map

map <pid_t, process> ProcessMap;		//combining the above 2 maps

queue<node*> ClockQueue;
queue<unsigned int> FreePhysMem;
queue<unsigned int> FreeDiskBlocks;

/***************************************************************************/
/* prototypes */
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

	//initializing array of phys mem
	// PhysMemP = new (nothrow) bool [PhysicalMemSize];
	// if (PhysMemP == NULL){
	// 	exit(FAILURE);
	// }

	//initializing free physical memory list
	for (unsigned int i = 0; i < PhysicalMemSize; i++){
		// cerr << "pushing " << i << " to memory free" << endl;
		FreePhysMem.push(i);
	}

	// for (unsigned int i = 0; i < PhysicalMemSize; i++){
	// 	PhysMemP[i] = false;
	// }

	//initializing array of disk blocks
	// DiskBlocksP = new (nothrow) bool [DiskSize];
	// if (DiskBlocksP == NULL){
	// 	exit(FAILURE);
	// }
	// for (unsigned int i = 0; i < DiskSize; i++){
	// 	DiskBlocksP[i] = false;
	// }

	//initializing free disk blocks list
	for (unsigned int i = 0; i < DiskSize; i++){
		FreeDiskBlocks.push(i);
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
	//initializing everything in the page table by setting the read & write 
	//bit to 0 and 1
	for (unsigned int i = 0; i < PAGE_TABLE_SIZE; i++){
		pointer->ptes[i].read_enable = 0; 
		pointer->ptes[i].write_enable = 0; 
	}

	// cerr << "here?" << endl;

	process proInfo;
	proInfo.lowestValidAddr = INVALID;
	proInfo.pageTableP = pointer;

	ProcessMap.insert(pair<pid_t, process> (pid, proInfo));

	//creating a new entry in PTMap
	//PTMap.insert(pair<pid_t, page_table_t*> (pid, pointer));

	//creating a new entry in AppArenaMap
	//AppArenaMap.insert(pair<pid_t, unsigned long int> (pid, INVALID));

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
	//// cerr << "switching to: " << pid << endl;
	// update the currentPid to the pid of the switched-to process
	CurrentPid = pid;

	page_table_base_register = ProcessMap[CurrentPid].pageTableP;

	//pointing the current pointer to the page table of switched-to process
	//page_table_base_register = PTMap[CurrentPid];
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
	//// cerr << "virtual address: " << virtualAddr << endl;
	// // cerr << "app arena current pid: " << hex << AppArenaMap[currentPid] << endl;
	//faulting on an invalid address
	// if (virtualAddr >= AppArenaMap[CurrentPid]) {
	// 	return FAILURE; //-1
	// }

	if (virtualAddr >= ProcessMap[CurrentPid].lowestValidAddr){
		return FAILURE;
	}

	//calculating page number associated with virtual address
	unsigned int pageNumber = virtualAddr / VM_PAGESIZE;
	// cerr << "virtual page number: " << hex << pageNumber << endl;
	//int pageOffSet = virtualAddr % VM_PAGESIZE;
	// cerr << "virtual page offset: " << pageOffSet << endl;
	page_table_entry_t* tempEntry = &(page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE]);
	// cerr << "what I am looking at " << tempEntry << endl;
	// cerr << "physical page number: " << hex << tempEntry->ppage << endl;
	// unsigned int ppageNumber = tempEntry->ppage;

	int nextPhysMem = 0;
	//if the page is not in resident (i.e not in physical memory)
	if (!resident(pageNumber)){
		// cerr << "not resident" << endl;
		nextPhysMem = nextAvailablePhysMem();
		// cerr << "next availabe physical memory is: " << nextPhysMem << endl;
		//there is no free physical memory, so have to run second-chance clock 
		//algorithm to evict active pages
		if (nextPhysMem == NO_VALUE){
			// node* curr = ClockQueue.front();
			// ClockQueue.pop();	
			while (true){
				// cerr << "size of queue: " << ClockQueue.size() << endl;
				node* curr = ClockQueue.front();
				ClockQueue.pop();	
				if (curr->refBit == 0){
					int evictedPage = curr->pageTableEntryP->ppage;
					// cerr << "evicting physical page: " << evictedPage << endl;

					//evicting the page of curr so set both read and write to 0
					// curr->refBit = 0;
					// curr->zeroFilledbit = 0;
					curr->pageTableEntryP->read_enable = 0;
					curr->pageTableEntryP->write_enable = 0;

					//updating residency of evicted page to false
					(DiskBlockMap[curr->vPage])[curr->pid].resident = false;

					//replacing that page with the current page
					tempEntry->ppage = evictedPage;

					//write to swap space if the evicted page has been modified
					//bool filledStatus = DiskBlockMap[curr->vPage][curr->pid].zeroFilledbit;
					
					//has never been modified before
					//if (!filledStatus){
						if (curr->modBit == 1){
							disk_write(curr->diskBlock, evictedPage);
							//DiskBlockMap[curr->vPage][curr->pid].zeroFilledbit = 1;	
							//zeroFill(evictedPage);	
							//DiskBlocksP[nextBlock] = true;
							//DiskBlockMap[evictedPage] = nextBlock;
						}
					//}
					// else {
					// 	//clear the memory the evicted page consumes
					// 	zeroFill(evictedPage);
					// }

					zeroFill(evictedPage);
					
					//the faulted page is already on disk, restore the information
					if (DiskBlockMap.count(pageNumber) > 0){
						if ((DiskBlockMap[pageNumber]).count(CurrentPid) > 0){
							//if ((DiskBlockMap[pageNumber])[CurrentPid].zeroFilledbit == 1){
							disk_read((DiskBlockMap[pageNumber])[CurrentPid].diskBlock, evictedPage);
							//}
						}
					}

					//creating a new entry for the queue for replacement	
					node* newNode = new (nothrow) node;
					newNode->pageTableEntryP = tempEntry;
					newNode->diskBlock = (DiskBlockMap[pageNumber])[CurrentPid].diskBlock;
					newNode->pid = CurrentPid;
					newNode->vPage = pageNumber;
					//newNode->zeroFilledbit = 1;					
					newNode->refBit = 1;

					if (write_flag == true){
						tempEntry->write_enable = 1;
						tempEntry->read_enable = 1;
						newNode->modBit = 1;
					}
					else {
						tempEntry->write_enable = 0;
						tempEntry->read_enable = 1;
						newNode->modBit = 0;
					}

					//updating queue and physical memory map
					ClockQueue.push(newNode);

					//update residency of page being put to memory to true
					(DiskBlockMap[pageNumber])[CurrentPid].resident = true;

					// PhysMemP[evictedPage] = true;
					PhysMemMap[evictedPage] = newNode;

					break;
				}
				else {
					curr->refBit = 0;
					curr->pageTableEntryP->read_enable = 0;
					curr->pageTableEntryP->write_enable = 0;
					ClockQueue.push(curr);
				}
			}
		}
		//there is unused physical memory, then just associate the page with that memory
		else {
			// // cerr << "there is extra memory" << endl;
			// // cerr << "assigned physical page: " << tempEntry->ppage << endl;
			//create a node in the memory
			node* nodeCreate = new (nothrow) node;
			tempEntry->ppage = nextPhysMem;
			tempEntry->read_enable = 1;
			tempEntry->write_enable = 0;
			nodeCreate->refBit = 1;
			nodeCreate->modBit = 0;
			if (write_flag == true){
				tempEntry->write_enable = 1;
				nodeCreate->modBit = 1;
			}
			nodeCreate->vPage = pageNumber;
			nodeCreate->pid = CurrentPid;
			nodeCreate->diskBlock = (DiskBlockMap[pageNumber])[CurrentPid].diskBlock;
			// cerr << "no memory with block " << nodeCreate->diskBlock << endl;
			nodeCreate->pageTableEntryP = tempEntry;

			//zero-filling for association (confused)
			// for (unsigned int i = nextPhysMem; i < (unsigned int)nextPhysMem + VM_PAGESIZE; i++){
			// 	((char*)pm_physmem)[i] = '\0';
			// }
			zeroFill(nextPhysMem);
			//nodeCreate->zeroFilledbit = 1;

			//stick the node to the end of clock queue
			ClockQueue.push(nodeCreate);

			//update to true for page in memory
			(DiskBlockMap[pageNumber])[CurrentPid].resident = true;

			// PhysMemP[nextPhysMem] = true;
			PhysMemMap[nextPhysMem] = nodeCreate;
		}
	}
	//the page is resident, update the bits, change the protections
	else {
		unsigned int tempPhysPage = tempEntry->ppage;
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
	// cerr << "destroying pid: " << CurrentPid << endl;
	//delete page_table_base_register;
	page_table_base_register = NULL;
	// AppArenaMap.erase(CurrentPid);
	// PTMap.erase(CurrentPid);
	map<unsigned int, node*>::iterator it = PhysMemMap.begin();

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

	//delete in physical memory
	unsigned int ppageDelete;
	unsigned int toDeleteVPage;
	for (it = PhysMemMap.begin(); it != PhysMemMap.end(); ){
		toDelete = it->second;
		toDeleteVPage = toDelete->vPage;
		if (toDelete->pid == CurrentPid){
			ppageDelete = toDelete->pageTableEntryP->ppage;
			if (toDelete->diskBlock != NO_VALUE){
				// cerr << "setting block " << toDelete->diskBlock << endl;
				//DiskBlocksP[toDelete->diskBlock] = false;
				FreeDiskBlocks.push((DiskBlockMap[toDeleteVPage])[CurrentPid].diskBlock);
				FreePhysMem.push(ppageDelete);
				DiskBlockMap[toDeleteVPage].erase(CurrentPid);
			}		
			// for (unsigned int i = 0; i < VM_PAGESIZE; i++){
			// 	((char*)pm_physmem)[ppageDelete * VM_PAGESIZE + i] = '\0';
			// }
			zeroFill(ppageDelete);
			//PhysMemP[ppageDelete] = false;
			PhysMemMap.erase(it++);
		}
		else {
			++it;
		}
	}

	//delete in disk block
	// cerr << "finished destroying " << CurrentPid << endl;
	// cerr << "============================================" << endl;
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

	//not enough disk blocks to back up
	// if (FreeDiskBlocks.empty()){
	// 	return NULL;
	// }

	int diskBlock = nextAvailableDiskBlock();
	if (diskBlock == NO_VALUE){
		return NULL;
	}

	//unsigned long nextLowest = AppArenaMap[CurrentPid];

	unsigned long nextLowest = ProcessMap[CurrentPid].lowestValidAddr;

	//starting at base
	if (nextLowest == INVALID){
		nextLowest = (unsigned long)VM_ARENA_BASEADDR;
	}
	//not enough arena size
	else if (nextLowest == (unsigned long)VM_ARENA_BASEADDR + VM_ARENA_SIZE){ 
		return NULL;
	}

	//// cerr << "returning " << hex << nextLowest << endl;

	//getting the page that corresponds to the valid address
	unsigned int validPageNum = nextLowest / VM_PAGESIZE;

	//after that, update the next highest valid bit
	//AppArenaMap[CurrentPid] = nextLowest + VM_PAGESIZE;
	ProcessMap[CurrentPid].lowestValidAddr = nextLowest + VM_PAGESIZE;

	//assign a specific disk block to any page number, and this will ALWAYS stay constant
	// cerr << "assiging block " << diskBlock << " to " << validPageNum << " of " << CurrentPid << endl;
	DiskBlockMap[validPageNum][CurrentPid].diskBlock = diskBlock;
	// DiskBlockMap[validPageNum] = diskBlock;

	//initializing the entry in page table
	//find the page table entry that corresponds to the lowest virtual address
	//set the read bit and write bit to 0 --> unused page
	//need pointer so we will change the value in the actual page table, not just the copy
	// page_table_entry_t* tempEntry = &(page_table_base_register->ptes[validPageNum]);
	// tempEntry->read_enable = 0;
	// tempEntry->write_enable = 0;
	// tempEntry->ppage = PhysicalMemSize + 1;

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

	//// cerr << hex << "syslonging: " << currAddr << endl;

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
		
		// if (tempEntry->read_enable == 0 && tempEntry->write_enable == 0){
		// 	return FAILURE;
		// }
		// if (tempEntry->zeroFilledbit == 0){
		// 	return FAILURE;
		// }

		pAddr = tempEntry->ppage * VM_PAGESIZE;

		//constructing the string
		s += string(1,((char*)pm_physmem)[pAddr + pageOffSet]);

		//s.append(1, ((char*)pm_physmem)[pAddr + pageOffSet]);

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
	// for (unsigned int i = 0; i < PhysicalMemSize; i++){
	// 	if (PhysMemP[i] == false){
	// 		// cerr << "returning available: " << i << endl;
	// 		return i;
	// 	}
	// }
	// return NO_VALUE;

	if (FreePhysMem.empty()){
		return NO_VALUE;
	}
	else {
		int avaiPhysMem = FreePhysMem.front();
		FreePhysMem.pop();
		//// cerr << "size of free memory: " << FreePhysMem.size() << endl;
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
	// for (unsigned int i = 0; i < DiskSize; i++){
	// 	if (DiskBlocksP[i] == false){
	// 		return i;
	// 	}
	// }
	if (FreeDiskBlocks.empty()){
		return NO_VALUE;
	}
	else {
		int avaiDisk = FreeDiskBlocks.front();
		FreeDiskBlocks.pop();
		//cerr << "next block returning " << avaiDisk << endl;
		return avaiDisk;
	}
	//return NO_VALUE;
}

/***************************************************************************
 Function: resdient
 Inputs:   none
 Returns:  true if the page is resident
 Description:
	
 ***************************************************************************/
bool resident(unsigned int vPage){
	// if (PhysMemMap[ppageNumber] != NULL){
	// 	return true;
	// }
	// else {
	// 	return false;
	// }

	return (DiskBlockMap[vPage])[CurrentPid].resident;

	//return (entry->zeroFilledbit == 1);
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

	//memset(&(((char*)pm_physmem)[ppage]), 0, VM_PAGESIZE);

}


/****** END OF FILE ********************************************************/