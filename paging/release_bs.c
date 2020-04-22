#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
   	STATWORD ps;
	disable(ps);
	bsd_t store = bs_id;
	int pid = currpid;	
	int k = 0, done = 0 , i = 0, j = 0; 
	if (bs_id < 0 || bs_id > NBSM)
	{
		kprintf("Release BS - Illegal Values !\n");
   		restore(ps);
		return SYSERR;
	}
	if (bsm_tab[store].bs_pid == pid && bsm_tab[store].bs_privHeap == 1)
        {
                /* Clear Frames and Pages used in BSMi and then Free BSM */
                clearFrameEntry (store , pid)   ;
		free_bsm(store);
                restore(ps);
                return OK;
        }

	else if ( bsm_tab[store].bs_privHeap == 1 && bsm_tab[store].bs_pid != pid)
        {
                kprintf("Release BS - PRIVATE HEAP ERROR !\n");
		restore(ps);
                return SYSERR;
        }
	
	int FoundPIDinBSM = 0;
        for ( i = 0; i < bsm_tab[store].bs_sharedProcCnt; i++)
        {
		//kprintf("Release BS - PID in BSM = %d at index %d !\n", bsm_tab[store].bs_sharedPID[i], i);

                if (bsm_tab[store].bs_sharedPID[i] == pid)
                {
		        FoundPIDinBSM = 1;
			break;
		}	
        }
	//kprintf("Release BS - Found in BSM = %d !\n", FoundPIDinBSM);	
	if (FoundPIDinBSM == 1)	
        {
                k = 0, done = 0 , i = 0, j = 0; 
                do
                {
			/* If it is the only one using the BSM - treat it like Private Heap with no lookup */
			if (bsm_tab[store].bs_sharedProcCnt == 1)
			{
				//kprintf("Release BS - FoundinBSM , SharedCount = 1 !\n");
				free_bsm(store);
				bsm_tab[store].bs_sharedProcCnt = 0;
				restore(ps);
				return OK;
			}

			/* If not , then clear the frames and move the proctab entries around */
			//clearFrameEntry (store , pid)   ;
			int indexForPID = -1;

			for (i = 0; i < bsm_tab[store].bs_sharedProcCnt; i++)
			{
				if ( pid == bsm_tab[store].bs_sharedPID[i])
				{
					indexForPID = i;
					break;
				}
			}

			 for ( i = indexForPID + 1; i < proctab[pid].sharedBSCount; i++)
			{

				proctab[currpid].sharedstore[i - 1] =  proctab[currpid].sharedstore[i];
				proctab[currpid].sharedvhpno[i -1] = proctab[currpid].sharedvhpno[i];
				proctab[currpid].sharedvhpnpages [i -1] = proctab[currpid].sharedvhpnpages[i];

			}
			bsm_tab[store].bs_sharedProcCnt--;
			done = 1;
			j++;
                }while ( j < bsm_tab[store].bs_sharedProcCnt && !done );
                restore(ps);
                return OK;
        }
	else if (bsm_tab[store].bs_status == BSM_UNMAPPED)
	{
		bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
		bsm_tab[bs_id].bs_pid = -1;
		bsm_tab[bs_id].bs_vpno = 0; /* Starting VP number */
		bsm_tab[bs_id].bs_npages = 0;
		bsm_tab[bs_id].bs_sem = 0;
		bsm_tab[bs_id].bs_privHeap = 0;

		/* Shared BSM Mechanism */
		bsm_tab[bs_id].bs_sharedProcCnt = 0;
		restore(ps);
		return OK;
	}
	else if (bsm_tab[store].bs_sharedProcCnt > 0 )
	{	/* proc might have been unmapped which would have deleited from the mapping */
		restore(ps);
		return OK;

	}	
	//kprintf("Release BS - PID not found in BSM !\n");		
	restore(ps);
	return SYSERR;

}

