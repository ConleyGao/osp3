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


/***************************************************************************/
/* constants */

using namespace std;

/***************************************************************************/
/* structs */


/***************************************************************************/
/* globals variables */
static pid_t currentPid = 0; 			//pid of currently running process

/***************************************************************************/
/* prototypdes */



/***************************************************************************
 Function: vm_init
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//venecia
void vm_init(unsigned int memory_pages, unsigned int disk_blocks);

/***************************************************************************
 Function: vm_create
 Inputs:   
 Returns:  nothing
 Description:
	
 ***************************************************************************/
//son
void vm_create(pid_t pid){
	assume that we have a global map name PtMap <pid, unsigned int *pointer>
	pid is the process id and a pointer to its own page table. This pointer 
	will be initialized to null

	pointer = new (nothrow) unsigned long int;
	if (pointer == NULL){
		exit(1);
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