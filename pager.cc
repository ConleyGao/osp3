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
static const unsigned long int INVALID = 0x00000000;
static const int NO_VALUE = -1;

using namespace std;

/***************************************************************************/
/* structs */
typedef struct {
	page_table_entry_t* pageTableEntryP;	//points to the entry in the corresponding page table
	unsigned long vpage;			//virtual page number
	pid_t pid; 					//which process this node belongs to
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
unsigned long pageTranslate(unsigned long vpage);

/***************************************************************************
 Function: vm_init
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//venecia
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
	DiskBlocksP = new (nothrow) bool [disk_blocks];
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
 Inputs:   
 Returns:  nothing
 Description:
	
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
 Inputs:   
 Returns:  nothing
 Description:
	
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
 Returns:  nothing
 Description:
	
 ***************************************************************************/
int vm_fault(void *addr, bool write_flag){

	unsigned long int virtualAddr = (long) addr;
	// cout << "virtual address: " << hex << virtualAddr << endl;
	// cout << "app arena current pid: " << hex << AppArenaMap[currentPid] << endl;
	//faulting on an invalid address
	if (virtualAddr > AppArenaMap[CurrentPid]) {
		return FAILURE; //-1
	}

	//calculating page number associated with virtual address
	int pageNumber = virtualAddr / VM_PAGESIZE;
	cout << "virtual page number: " << pageNumber << endl;
	int pageOffSet = virtualAddr % VM_PAGESIZE;
	cout << "virtual page offset: " << pageOffSet << endl;
	page_table_entry_t* tempEntry = &(page_table_base_register->ptes[pageNumber - VM_ARENA_BASEPAGE]);
	cout << "ppage: " << hex << tempEntry->ppage << endl;
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
					int availPhysPage = curr->pageTableEntryP->ppage / VM_PAGESIZE;
					//if not last accessed, page out
					//PhysMemMap.erase((curr->pageTableEntryP->ppage)/VM_PAGESIZE);
					int nextBlock = nextAvailableDiskBlock();
					if (nextBlock == NO_VALUE){
						return FAILURE;
					}

					//write to swap space if page has been modified
					if (curr->modBit == 1){
						disk_write(nextBlock, curr->pageTableEntryP->ppage);
						DiskBlocksP[nextBlock] = true;
					}

					//creating a new entry for the queue for replacement
					node* newNode;
					newNode->pageTableEntryP = tempEntry;
					newNode->modBit = 0;
					//refBit should be 1 because accessed
					newNode->refBit = 1;
					newNode->vpage = virtualAddr;

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
			cout << "there is extra memory" << endl;
			tempEntry->ppage = nextPhysMem;
			cout << "ppage: " << hex << tempEntry->ppage << endl;
			//tempEntry->read_enable = 1;
			if (write_flag == true){
				tempEntry->write_enable = 1;
			}
			else {
				tempEntry->read_enable = 1;
			}
			
			//create a node in the memory
			node* nodeCreate;
			nodeCreate->modBit = 0;
			if (write_flag == true){
				nodeCreate->modBit = 1;
			}
			nodeCreate->refBit = 1;
			nodeCreate->vpage = virtualAddr;
			nodeCreate->pid = CurrentPid;
			nodeCreate->pageTableEntryP = tempEntry;

			//stick the node to the end of clock queue
			ClockQueue.push(nodeCreate);
			cout << "size of clock queue: " << ClockQueue.size() << endl;
			// if (head->next == NULL){
			// 	head->next = nodeCreate;
			// 	nodeCreate->next = head;
			// 	tail = nodeCreate;
			// }
			// else {
			// 	tail->next = nodeCreate;
			// 	tail = nodeCreate;
			// 	nodeCreate->next = head;
			// }

			PhysMemP[nextPhysMem] = true;
			PhysMemMap.insert(pair<unsigned int, node*>(nextPhysMem, nodeCreate));
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
 Inputs:   
 Returns:  nothing
 Description:
	Called when current process exits
 	Deallocate all resources held by the current process 
 	(page table, physical pages, disk blocks, etc.)
 ***************************************************************************/
void vm_destroy(){
	delete page_table_base_register;
	AppArenaMap.erase(CurrentPid);
	PTMap.erase(CurrentPid);
	map<unsigned int, node*>::iterator it = PhysMemMap.begin();
	while (it != PhysMemMap.end()){
		node* toDelete = it->second;
		if (toDelete->pid == CurrentPid){
			PhysMemP[toDelete->pageTableEntryP->ppage / VM_PAGESIZE] = false;
			PhysMemMap.erase(it++);
		}
		else {
			it++;
		}
	}
}

/*
	i have no parameters so i believe that i will be dealing with 
	instances of page_table_entry_t; and
	typedef struct {
    	page_table_entry_t ptes[VM_ARENA_SIZE/VM_PAGESIZE];
	} page_table_t;

	also other structs that we create would need to be deallocated

	very dependent

	use delete
*/

/***************************************************************************
 Function: vm_extend
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//together
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

	cout << "returning " << hex << nextLowest << endl;

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
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
int vm_syslog(void *message, unsigned int len){

	if (len == 0 || len > (AppArenaMap[CurrentPid] - (unsigned long)VM_ARENA_BASEADDR)){
		return FAILURE;
	}

	string s;

	for (unsigned int i = 0; i < len; i++){
		s += string(1,((char*)pm_physmem)[(page_table_base_register->ptes[((unsigned long)VM_ARENA_BASEADDR + (unsigned long)i) / VM_PAGESIZE - VM_ARENA_BASEPAGE]).ppage]);
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
	if (PhysMemMap.count(ppageNumber) > 0){
		if (PhysMemMap[ppageNumber] != NULL){
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
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
unsigned long pageTranslate(unsigned long vpage);






/****** END OF FILE *********************************************************/