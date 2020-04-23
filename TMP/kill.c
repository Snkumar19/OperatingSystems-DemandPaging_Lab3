/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	
	//kprintf (" kill called for pid: %d\n", pid);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	/* Process Destruction */
	int i = 0, j = 0, done = 0;
		
	if (proctab[pid].store != -1)
	{
		// This means Process had a Private Heap
		bsm_unmap (pid, bsm_tab[proctab[pid].store].bs_vpno, 1);
	}
	if (proctab[pid].sharedBSCount != 0)
	{
		// If the process has any shared BS other than Private Heap
		for ( i = 0; i < proctab[pid].sharedBSCount; i++ )
		for ( j = 0; j < bsm_tab[proctab[pid].sharedstore[i]].bs_sharedProcCnt; j ++)
		{
			if ( bsm_tab[proctab[pid].sharedstore[i]].bs_sharedPID[j] == pid)
			{
				
				bsm_unmap (pid, bsm_tab[proctab[pid].sharedstore[i]].bs_sharedVPNO[j], 0); 	
			}
		}
	}
	
	for ( i = 0 ; i < NBSM ; i++)
		if (bsm_tab[i].bs_status == BSM_UNMAPPED )
		{
			if ( bsm_tab[i].bs_sharedProcCnt != 0)
			 	bsm_tab[i].bs_sharedProcCnt = 0;
			if (  bsm_tab[i].bs_privHeap == 1)
				 bsm_tab[i].bs_privHeap = 0;	
		}
	for ( i = 0; i < NFRAMES; i++)
	{
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE)
		{
			free_frm(i);
		}
	}

	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
