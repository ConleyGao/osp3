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

typedef struct {
	unsigned long int* areanP;
	unsigned long int lowestValid;
} arenaEntry;

/***************************************************************************/
/* globals variables */
static unsigned int availableDisks = 0;
static pid_t currentPid = 0; 					//pid of currently running process
static node* head;
static node* tail;
static node* curr;
static unsigned long int *CurrAppArena;			//points to the current arena
static unsigned int PhysicalMemSize;			//size of physical memory
static bool* PhysMemP;
map <pid_t, arenaEntry> AppArenaMap;			//arena map
map <pid_t, page_table_t*> PTMap;				//page table map
map <unsigned int, bool> PhysMemMap;

/***************************************************************************/
/* prototypdes */
unsigned long pageTranslate(unsigned long vpage){

}


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

	CurrAppArena = new unsigned long int [VM_ARENA_SIZE];

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

	unsigned long int *pointer = new (no throw) unsigned long int [VM_ARENA_SIZE];
	if (pointer == NULL){
		exit(1);
	}

	for (unsigned long int i = 0; i < VM_ARENA_SIZE; i++)}{
		pointer[i] = INVALID;
	}

	arenaEntry toCreate;
	toCreate.areanP = pointer;
	toCreate.lowestValid = 0;

	AppArenaMap.insert< pair(<pid_t, arenaEntry> (pid, toCreate));

}

/***************************************************************************
 Function: vm_switch
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//son
void vm_switch(pid_t pid){
	currentPid = pid;
	currAppArena = AppArenaMap[pid].areanP;
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

	unsigned long int nextLowest = AppArenaMap[currentPid].lowestValid + 1;

	for (unsigned long int i = nextLowest; i < nextLowest + VM_PAGESIZE; i++){
		AppArenaMap[currentPid].arenaP[i] = 0;
	}

	node* nodeCreate;
	nodeCreate->PageTableP = new (nothorw) page_table_t;
	nodeCreate->modBit = 0;
	nodeCreate->refBit = 0;
	nodeCreate->next = NULL; 

	tail->next = nodeCreate;
	tail = nodeCreate;

	page_table_entry_t tempEntry = nodeCreate->PageTableP->ptes[nextLowest/VM_PAGESIZE];
	unsigned long int mapPPage = nextAvailablePhysMem();

	if (mapPPage == INVALID) {
		return NULL;
	}

	PhysMemP[mapPPage] = true;

	tempEntry.ppage = mapPPage;
	tempEntry.read_enable = 1;
	tempEntry.write_enable = 1;

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

unsigned long int nextAvailablePhysMem(){
	for (unsigned long int i = 0; i < PhysicalMemSize; i++){
		if (PhysMemP[i] == false){
			return i;
		}
	}
	return INVALID;
}








/****** END OF FILE *********************************************************/