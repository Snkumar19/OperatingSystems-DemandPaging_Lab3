#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
	
    	/* requests a new mapping of npages with ID map_id */
    	STATWORD ps;
	disable(ps);
	if ( bs_id < 0 || bs_id > NBSM || npages <= 0 || npages > 128)	
	{
		kprintf("Error : GET BS - Illegal values !\n");
    		restore(ps);
		return SYSERR;
    
	}

	if ((bsm_tab[bs_id].bs_status == BSM_MAPPED && bsm_tab[bs_id].bs_privHeap == 1))
	{
		kprintf("Error : GET BS -  Illegal Mapping to Private Heap !\n");
                restore(ps);
                return SYSERR;

	}
	/* If BSM is unmapped or, if BSM is mapped and not private heap then Map it */
	if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED || (bsm_tab[bs_id].bs_status == BSM_MAPPED && bsm_tab[bs_id].bs_privHeap == 0) )
	{
		if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
			bsm_tab[bs_id].bs_sharedProcCnt = 0;	
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_sharedPID[bsm_tab[bs_id].bs_sharedProcCnt] = currpid;
		bsm_tab[bs_id].bs_sharedNPages[bsm_tab[bs_id].bs_sharedProcCnt] = npages;
		//kprintf ("GET BS for %d - Shared PID : %d \n",bs_id, bsm_tab[bs_id].bs_sharedPID[bsm_tab[bs_id].bs_sharedProcCnt]);
		//kprintf ("GET BS for %d - Shared NPAGES : %d \n",bs_id, bsm_tab[bs_id].bs_sharedNPages[bsm_tab[bs_id].bs_sharedProcCnt]);
		
		bsm_tab[bs_id].bs_sharedProcCnt++;
		//kprintf ("GET BS for %d - Shared Count : %d \n",bs_id, bsm_tab[bs_id].bs_sharedProcCnt);
		
		restore(ps);
		return bsm_tab[bs_id].bs_sharedNPages[0];
	}

	restore(ps);
	return SYSERR;

}


