/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps);

	int i = 0;

	//kprintf("\n Init BSM");
	for (i = 0; i < NBSM; i++)
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 0; /* Starting VP number */
		bsm_tab[i].bs_npages = 0;
                bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_privHeap = 0;
	}

	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
        disable(ps);

        int i = 0;

	for (i = 0; i < NBSM; i++)
        {
		/* Find the first Unmapped Backing store */
        	if( bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			*avail = i;
			restore(ps);
			return (OK);
		}
	}
	/* no free backing store ID - return error */
	restore(ps);
	return (SYSERR);
					
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
        disable(ps);

	bsm_tab[i].bs_status = BSM_UNMAPPED;
        bsm_tab[i].bs_pid = -1;
        bsm_tab[i].bs_vpno = 0; /* Starting VP number */
        bsm_tab[i].bs_npages = 0;
        bsm_tab[i].bs_sem = 0;
	bsm_tab[i].bs_privHeap = 0;
	restore(ps);
        return(OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	
	/* Find PD and page number from Virtaul address - right shift by 12 (4k))*/

	int virtpage = 0;
	int i =0;
	
	virtpage = vaddr >> 12;

	for (i = 0; i < NBSM; i++)
        {
		if(bsm_tab[i].bs_pid == pid)
		{
			*pageth = virtpage - bsm_tab[i].bs_vpno;
			*store = i;
			return (OK);
		}
	}	
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD ps;
        disable(ps);
	/*	
	if ( vpno > 128 || vpno < 0 )
	{
		restore(ps);
		return SYSERR;
	}	
	*/
	bsm_tab[source].bs_status = BSM_MAPPED;
        bsm_tab[source].bs_pid = pid;
        bsm_tab[source].bs_vpno = vpno;
        bsm_tab[source].bs_npages = npages;
        bsm_tab[source].bs_sem = 0;

       	proctab[pid].store = source;
	proctab[pid].vhpno = vpno;
	 
	restore(ps);
        return(OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
        disable(ps);

	/* Use VPNO to calcuate VA, to lookup in the bsm_lookup. 
	 * If FRM matches PID and it's FR_DIR -> Then Update pageth using bsm_lookup and 
 	 * parse it to write_bs before unmapping it 
 	*/
	int i = 0;
		
	/*	
	for ( i = 0; i < NFRAMES; i++){
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_DIR){
			int page, src;
			bsm_lookup( pid , (vpno << 12) , &src, &page);
			write_bs ( ((i + NFRAMES) << 12, src, page);
		}
	} 

	*/

	/* UNMAP BACKING STORE */ 	
	int source = proctab[pid].store;
        
	bsm_tab[source].bs_status = BSM_UNMAPPED;
        bsm_tab[source].bs_pid = -1;
        bsm_tab[source].bs_vpno = 0; /* Starting VP number */
        bsm_tab[source].bs_npages = 0;
        bsm_tab[source].bs_sem = 0;
	bsm_tab[source].bs_privHeap = 0;
        restore(ps);
        return(OK);
}


