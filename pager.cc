/****************************************************************************
 File:   pager.cc
 Author: Adela Yang and Son Ngo and Venecia Xu
 Date:   Nov 2015

 Description:

 Notes: 
 Run this program with argument on the command line
 	
 Data Structures:

   
******************************************************************************/

/*****************************************************************************/
/* include files */
#include <cstdlib>
#include <iostream>
#include <vm_pager.h>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

/***************************************************************************/
/* constants */
static const int FAILURE = -1;
static const int SUCCESS = 0;
static const unsigned long int INVALID = 0x00000000;

using namespace std;

/***************************************************************************/
/* structs */
typedef struct {
	page_table_t* pageTableP; 
	unsigned int modBit;
	unsigned int refBit;
	node *next;
} node;

// typedef struct {
// 	unsigned long int* areanP;
// 	unsigned long int lowestValid;
// } arenaEntry;

/***************************************************************************/
/* globals variables */
static unsigned int availableDisks = 0;
static pid_t currentPid = 0; 					//pid of currently running process
static node* head;
static node* tail;
static node* curr;
//static unsigned long int *CurrAppArena;			//points to the current arena
static unsigned int PhysicalMemSize;			//size of physical memory
static bool* PhysMemP;
//map <pid_t, arenaEntry> AppArenaMap;			//arena map
map <pid_t, unsigned long int> AppArenaMap;
map <pid_t, page_table_t*> PTMap;				//page table map
map <unsigned int, bool> PhysMemMap;

static unsigned long int NextLowestValidVP;		//next lowest valid virtual address in the app's arena

/***************************************************************************/
/* prototypes */
unsigned long newAvailablePhysMem();
unsigned long pageTranslate(unsigned long vpage);

//arena  - just a set of nums, its arb
	//dont need pointer?
	//contains addresses
	// get the page and do something with it 
//tracking virtual pages, frames, processes




/***************************************************************************
 Function: vm_init
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//venecia
void vm_init(unsigned int memory_pages, unsigned int disk_blocks){

	head = new (nothrow) node;
	if (head == NULL){
		exit(FAILURE);
	}

	page_table_base_register = new (nothrow) page_table_t;
	if (page_table_base_register == NULL){
		exit(FAILURE);
	}

	availableDisks = disk_blocks;
	PhysicalMemSize = memory_pages;

	// CurrAppArena = new unsigned long int [VM_ARENA_SIZE];

	PhysMemP = new unsigned bool [PhysicalMemSize];
}

/***************************************************************************
 Function: vm_create
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//son
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

	// AppArenaMap.insert< pair(<pid_t, arenaEntry> (pid, toCreate));

}

/***************************************************************************
 Function: vm_switch
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
void vm_switch(pid_t pid){

	// update the currentPid to the pid of the switched-to process
	currentPid = pid;

	// points to arena pointer to the arena of the switched-to process
	// currAppArena = AppArenaMap[pid].areanP;

	//pointing the current pointer to the page table of switched-to process
	page_table_base_register = PTMap[pid];
}

/***************************************************************************
 Function: vm_fault
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//together
int vm_fault(void *addr, bool write_flag);

/***************************************************************************
 Function: vm_destroy
 Inputs:   
 Returns:  nothing
 Description:
	Called when current process exits
 	Deallocate all resources held by the current process 
 	(page table, physical pages, disk blocks, etc.)
 ***************************************************************************/
void vm_destroy();

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

	/* IDEA FOR THE CODE BELOW
	
	find the lowest using the variable associated with each arena (lowestValid)
	and then add either 1 or 1fff (I don't know). 

	map it to physical memory and update the page table (modifying ppage). 
	By "update the page table", I mean create a node, stick it to the head,
	and initialize values of modbit, refbit, read, write and ppage (mapping)
	
	go to physical memory and set the value to 0

	*/

	//check if having enough disk blocks to write if necessary
	if (availableDisks == 0){
		return NULL;
	}

	//getting the next lowest invalid address
	unsigned long int nextLowest = AppArenaMap[currentPid] + 1;

	//after that, update the next lowest invalid bit
	AppArenaMap[currentPid] += VM_PAGESIZE;

	// for (unsigned long int i = nextLowest; i < nextLowest + VM_PAGESIZE; i++){
	// 	AppArenaMap[currentPid].arenaP[i] = 0;
	// }

	//create a node in the memory
	node* nodeCreate;
	// nodeCreate->PageTableP = new (nothrow) page_table_t;
	nodeCreate->modBit = 0;
	nodeCreate->refBit = 0;
	nodeCreate->next = NULL; 

	tail->next = nodeCreate;
	tail = nodeCreate;

	//find the page table entry that corresponds to the lowest virtual address
	//set the read bit and write bit to 0 --> unused page
	//need pointer so we will change the value in the actual page table, not just the copy
	page_table_entry_t* tempEntry = &(nodeCreate->PageTableP->ptes[nextLowest/VM_PAGESIZE - VM_ARENA_BASEPAGE]);
	tempEntry->read_enable = 0;
	tempEntry->write_enable = 0;

	// unsigned long int mapPPage = nextAvailablePhysMem();
	// if (mapPPage == INVALID) {
	// 	return NULL;
	// }
	// PhysMemP[mapPPage] = true;
	// tempEntry.ppage = mapPPage;


	return nextLowest;
}

/***************************************************************************
 Function: vm_syslog
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//together
int vm_syslog(void *message, unsigned int len);


/***************************************************************************
	UTILITY FUNCTIONS
 ***************************************************************************/

/***************************************************************************
 Function: nextAvailablePhysMem
 Inputs:   none
 Returns:  nothing
 Description:
	
 ***************************************************************************/
unsigned long int nextAvailablePhysMem(){
	for (unsigned long int i = 0; i < PhysicalMemSize; i++){
		if (PhysMemP[i] == false){
			return i;
		}
	}
	return INVALID;
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