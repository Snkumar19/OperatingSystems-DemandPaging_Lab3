/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  	STATWORD ps;
	disable(ps);
	// kprintf("xmmap - to be implemented!\n");
	if (virtpage < 4096 || source < 0 || source > NBSM || npages > 128)
 	{
		kprintf("xmmap - illegal mapping values!\n");
		restore(ps);
		return SYSERR;
	}

        if ( bsm_tab[source].bs_status == BSM_UNMAPPED )
        {
                restore(ps);
                return SYSERR;
        }


        if ( bsm_tab[source].bs_privHeap == 1)
        {
		restore(ps);
                return SYSERR;
        }
        
       	int pid = currpid;
	int i = 0, index = 0;
	for (i = 0; i < bsm_tab[source].bs_sharedProcCnt; i++)
	{
		if (bsm_tab[source].bs_sharedPID[i] == currpid)
		{
			index = i;
			break;
		}
	} 
	bsm_tab[source].bs_sharedNPages[index] = npages;
	bsm_tab[source].bs_sharedVPNO[index] = virtpage;

	proctab[pid].sharedstore[proctab[pid].sharedBSCount] = source;
	proctab[pid].sharedvhpnpages[proctab[pid].sharedBSCount] = npages;
	proctab[pid].sharedBSCount++;


	restore(ps);
	return OK;

}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD ps;
        disable(ps);
        unsigned int i; 
	if (virtpage < 4096)
	{
		kprintf("xmunmap - illegal virtual page value!\n");
                restore(ps);
		return SYSERR;
        } 
	/* If the Frame belonging to PID, was used for Page Directory, lookup the page and 
 	 * write it back to Backing-Store 
 	 *  
 	 *  */
        /*
	for ( i = 0; i < NFRAMES; i++){
                if (frm_tab[i].fr_pid == currpid && frm_tab[i].fr_type == FR_DIR){
                        int page, src;
			// virtpage X 4k 
                        bsm_lookup( currpid , (virtpage << 12) , &src, &page);
			// Write PD to back to BS in case some process unmaps it 
                        write_bs ( ((i + NFRAMES) << 12), src, page);
                }
        } 
	*/
	if ( bsm_unmap(currpid, virtpage, 0) == OK )
	{
		restore(ps);
		return OK;
	}
	kprintf("xmunmap - BSM Unmap Failed !\n");
	restore(ps);
	return SYSERR; 
}
