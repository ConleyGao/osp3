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

/***************************************************************************/
/* constants */
static const int FAILURE = -1;
static const int SUCCESS = 0;
static const unsigned long int INVALID = 0x00000000;

using namespace std;

/***************************************************************************/
/* structs */
typedef struct node {
	page_table_entry_t
	unsigned int modBit;
	unsigned int refBit;
	node *next;
} node;

/***************************************************************************/
/* globals variables */
static pid_t currentPid = 0; 			//pid of currently running process
static node* head;
static unsigned long int *currAppArena;
map <pid_t, unsigned long int*> appArenaMap;

/***************************************************************************/
/* prototypdes */



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

	currAppArena = new unsigned long int [VM_ARENA_SIZE];

}

/***************************************************************************
 Function: vm_create
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//son
void vm_create(pid_t pid){

	pointer = new unsigned long int [VM_ARENA_SIZE];
	if (pointer == NULL){
		exit(1);
	}

	for (unsigned int i = 0; i < VM_ARENA_SIZE; i++)}{
		pointer[i] = INVALID;
	}

	PtMap.insert< pair(<int, unsigned long int*> (pid, pointer));

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
	currAppArena = PtMap[pid];
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
//adela
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
void * vm_extend();


/***************************************************************************
 Function: vm_syslog
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//together
int vm_syslog(void *message, unsigned int len);


/****** END OF FILE *********************************************************/