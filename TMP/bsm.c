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

	int i = 0, j = 0;

	//kprintf("\n Init BSM");
	for (i = 0; i < NBSM; i++)
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 0; /* Starting VP number */
		bsm_tab[i].bs_npages = 0;
                bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_privHeap = 0;
		
		/* Shared BSM Mechanism */
		bsm_tab[i].bs_sharedProcCnt = 0;
		
		for ( j = 0; j < NPROC; j++)
		{
			bsm_tab[i].bs_sharedPID[j] = -1;
			bsm_tab[i].bs_sharedVPNO[j] = 0;
			bsm_tab[i].bs_sharedNPages[j] = 0;

		}
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
	
	/* Shared BSM Mechanism */
	//bsm_tab[i].bs_sharedProcCnt = 0;
	int j = 0 ;
	for ( j = 0; j < NPROC; j++)
	{
		bsm_tab[i].bs_sharedPID[j] = -1;
		bsm_tab[i].bs_sharedVPNO[j] = 0;
		bsm_tab[i].bs_sharedNPages[j] = 0;

	}
	j = 0; 
	int k = 0, done = 0;
	do
	{
		if (proctab[currpid].sharedstore[j] == i)
		{
			int k = 0;
			/* Moving Proctab Entry */
			for ( k = j + 1; k < proctab[currpid].sharedBSCount; k ++)
			{
				proctab[currpid].sharedstore[k - 1] =  proctab[currpid].sharedstore[k];
				proctab[currpid].sharedvhpno[k -1] = proctab[currpid].sharedvhpno[k];
				proctab[currpid].sharedvhpnpages [k -1] = proctab[currpid].sharedvhpnpages[k];
			}
			done  = 1;
		}
		j++;
	}while ((j < proctab[currpid].sharedBSCount) && (!done));

        return(OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	
	/* Find PD and page number from Virtaul address - right shift by 12 (4k))*/

	int i =0, j = 0;
	
	
	/* Let's do a Private Heap Test - If the PID is using the backing store as a Private Heap*/ 

	for (i = 0; i < NBSM; i++)
        {
		if(bsm_tab[i].bs_pid == pid && bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_privHeap == 1  )
		{
			int virtpage = vaddr>>12;
			if (bsm_tab[i].bs_vpno <= virtpage && bsm_tab[i].bs_npages >= (virtpage - bsm_tab[i].bs_vpno))
                        {
				*pageth = (vaddr>>12) - bsm_tab[i].bs_vpno;
				*store = i;
				return (OK);
			}
			else
			{
				//kprintf ("BSM for Private Heap => Npages out of bounds\n");
				return SYSERR;
			}
		}
	}
	
	/* If the control is here, then the PID is not using it as a Private Heap
	 * and hence should be in the shared part of the BSM
	 */
	
	/* Shared BSM mode */
	
	if (proctab[pid].sharedBSCount == 0)
	{
	//	kprintf ("PID doesn't have any Backing store - Shared/PrivateHeap\n");
		return SYSERR;
	}

	for ( i = 0; i <  proctab[pid].sharedBSCount; i++ )
	{
		//kprintf ("  proctab[pid].sharedBSCount = %d \n",  proctab[pid].sharedBSCount);
		for ( j = 0; j < bsm_tab[proctab[pid].sharedstore[i]].bs_sharedProcCnt; j++)
		{
				if ( bsm_tab[proctab[pid].sharedstore[i]].bs_sharedPID[j] ==  pid )
				{	
				//virtpage = vaddr >> 12;
				//if ( bsm_tab[proctab[pid].sharedstore[i]].bs_sharedVPNO[j] <= virtpage 
				//	&& bsm_tab[proctab[pid].sharedstore[i]].bs_sharedNPages[j] >= (virtpage - bsm_tab[proctab[pid].sharedstore[i]].bs_sharedVPNO[j]))
				*pageth = (vaddr>>12) - bsm_tab[proctab[pid].sharedstore[i]].bs_sharedVPNO[j];
				*store =   proctab[pid].sharedstore[i];
				//kprintf ("BSM Returned => Store - %d, Pageth - %d \n", *store, *pageth);
				return (OK);
				}
		}
	}

	//kprintf ("ERROR : BSM => Problem is here!! \n");
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
		
	if (  bsm_tab[source].bs_status == BSM_MAPPED )
	{
		restore(ps);
		return SYSERR;
	}	
	
	bsm_tab[source].bs_status = BSM_MAPPED;

	if ( bsm_tab[source].bs_privHeap == 1)
	{
			
		bsm_tab[source].bs_pid = pid;
		bsm_tab[source].bs_vpno = vpno;
		bsm_tab[source].bs_npages = npages;
		bsm_tab[source].bs_sem = 0;
       		proctab[pid].store = source;
		proctab[pid].vhpno = vpno;
	}
	else
	{
		bsm_tab[source].bs_sharedPID[bsm_tab[source].bs_sharedProcCnt] = 1;
		bsm_tab[source].bs_sharedNPages[bsm_tab[source].bs_sharedProcCnt] = npages;
		bsm_tab[source].bs_sharedVPNO[bsm_tab[source].bs_sharedProcCnt] = vpno;
		bsm_tab[source].bs_sharedProcCnt++;

		proctab[pid].sharedstore[proctab[pid].sharedBSCount] = source;
		proctab[pid].sharedvhpno[proctab[pid].sharedBSCount] = vpno;
		proctab[pid].sharedBSCount++;
	}
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
	
	int i = 0, j = 0, pageth = 0, store = 0;
	if (proctab[pid].sharedBSCount != 0 )
	{
		for ( i = 0; i < proctab[pid].sharedBSCount ; i ++ )
		{
			if (bsm_lookup(pid, vpno << 12, &store, &pageth) != SYSERR)
			 	break;
			//else
			//	 kprintf ("ERROR: BSMUNMAP - Lookup Failed \n"); 
		}
	}

	if (bsm_tab[store].bs_status == BSM_UNMAPPED) 
	{
		//kprintf ("ERROR: BSMUNMAP - Unmapping a already unmapped BSM entry \n");
		restore(ps);
		return SYSERR;

	}
	
	if (bsm_tab[store].bs_pid == pid && bsm_tab[store].bs_privHeap == 1)
	{
		/* Clear Frames and Pages used in BSMi and then Free BSM */
		
		//kprintf ("BSMUNMAP - IF Condition \n");
		clearFrameEntry (store , pid)   ;
		free_bsm(store);
		restore(ps);
		return OK;
	}

	else if ( bsm_tab[store].bs_privHeap == 1 && bsm_tab[store].bs_pid != pid)
	{
		//kprintf ("BSMUNMAP - ELSE IF Condition \n");
		restore(ps);
		return SYSERR;
	}	
	/* UNMAP SHARED BACKING STORE */ 	
	int FoundPIDinBSM = 0;
	for ( i = 0; i < bsm_tab[store].bs_sharedProcCnt; i++)
	{
		if (bsm_tab[store].bs_sharedPID[i] == pid)
			 FoundPIDinBSM = 1;
	}
		
	if (FoundPIDinBSM ) 
	{
		int k = 0, done = 0 ;
		i = 0, j = 0;
		//kprintf ("BSMUNMAP - ELSE Condition \n");
		do
		{
			/* If it is the only one using the BSM - treat it like Private Heap */
			if (bsm_tab[store].bs_sharedProcCnt == 1)
			{
				
				//kprintf ("BSMUNMAP - SharedCnti == 1 Condition \n");
				clearFrameEntry (store , pid)	;
				free_bsm(store);
				//bsm_tab[store].bs_sharedProcCnt = 0;	
				restore(ps);
				return OK;
			}
			//kprintf ("BSMUNMAP - SharedCnt > 1 Condition \n");
			/* If not , then clear the frames and move the proctab entries around */
			clearFrameEntry (store , pid)   ;	
			int indexForPID = -1;

			for (i = 0; i < bsm_tab[store].bs_sharedProcCnt; i++)
			{				 	
				if ( pid == bsm_tab[store].bs_sharedPID[i])
				{
					indexForPID = i;
					break;
				}
			}
			delete_proctabEntryAtIndex (indexForPID, proctab[pid].sharedBSCount); 
			if (bsm_tab[store].bs_sharedProcCnt > 1)
				bsm_tab[store].bs_sharedProcCnt--;
			
			
			done = 1;	
			j++;
		}while ( j < bsm_tab[store].bs_sharedProcCnt && !done );
		restore(ps);
		return OK;
	}
	//kprintf ("BSMUNMAP => NotFoundPIDinBSM ");
	restore(ps);
	return SYSERR;
}

void delete_proctabEntryAtIndex (int index, int sharedCount) 
{
	int i = 0;
	for ( i = index + 1; i < sharedCount; i++ )
	{
		proctab[currpid].sharedstore[i - 1] =  proctab[currpid].sharedstore[i];
                proctab[currpid].sharedvhpno[i -1] = proctab[currpid].sharedvhpno[i];
                proctab[currpid].sharedvhpnpages [i -1] = proctab[currpid].sharedvhpnpages[i];
	}

}

void clearFrameEntry (int store, int pid)
{
	int i = 0;
	
	int frm_storeID = 0, frm_pageth = 0 ;
	for (i = 0; i < NFRAMES ; i++)
	{	
		frm_storeID = 0, frm_pageth = 0 ;
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE)
		{
			//kprintf (" ClearFrame Entry Calling - BSM LOOKUP %d %d\n", frm_storeID, frm_pageth);
			if (bsm_lookup(frm_tab[i].fr_pid , frm_tab[i].fr_vpno << 12, &frm_storeID, &frm_pageth) == OK) 
			{
				if (frm_storeID == store)
				{
					//kprintf (" ClearFrame Entry Calling - free_frm(%d)\n", i);
					free_frm (i);
				}
			}
		}
	}
}
