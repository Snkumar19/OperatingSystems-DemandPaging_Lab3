/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  /* sanity check ! */

  /*kprintf("To be implemented!\n");*/
	STATWORD ps;
	disable(ps);

	if(policy != SC && policy != LFU)
	{
		restore(ps);
		return (SYSERR);
	}

	page_replace_policy = policy;
	//kprintf("To be implemented!\n");
	restore(ps);
  	return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
	return page_replace_policy;
}
