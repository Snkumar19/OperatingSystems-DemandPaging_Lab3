#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
   	STATWORD ps;
	disable(ps);
	
	if (bs_id < 0 || bs_id > NBSM)
	{
		kprintf("Release BS - Illegal Values !\n");
   		restore(ps);
		return SYSERR;
	}

	free_bsm(bs_id);
	restore(ps);
	return OK;

}

