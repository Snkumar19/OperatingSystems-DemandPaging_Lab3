/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	// kprintf("To be implemented!\n");
	// Process is still created using create 
	// Get BSM as private heap
	// Then map the BSM to the process
	STATWORD ps;
	disable(ps);
	
	int userpid = create(procaddr,ssize,priority,name,nargs,args);
	int avail;			
	int bs_id = get_bsm(avail);	
	
	if (bs_id == SYSERR || hsize < 0 || hsize > 128 || userpid == SYSERR){
		kprintf("VCreate Error - Invalid BSM/HSize or PID !\n");	
		restore(ps);
		return SYSERR;
	}

	int bsm_val = bsm_map(userpid, 4096, avail, hsize);
	
	// BSM_MAP writes the values for store and vpno, we need to update bsm_tab for virtualHeap
	// ProcTab entry for vheapsize and update vmemlist
	//
	
	proctab[userpid].vhpnpages = hsize;
	proctab[userpid].vmemlist->mnext = 4096*NBPG;
	
	bsm_tab[avail].bs_privHeap = 1;
 	
	restore(ps);
	return OK;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
