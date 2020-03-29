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
 	bsm_map( currpid,  virtpage, source, npages) ;
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
        for ( i = 0; i < NFRAMES; i++){
                if (frm_tab[i].fr_pid == currpid && frm_tab[i].fr_type == FR_DIR){
                        int page, src;
                        bsm_lookup( currpid , (virtpage << 12) , &src, &page);
                        write_bs ( ((i + NFRAMES) << 12), src, page);
                }
        } 
	bsm_unmap(currpid, virtpage, 0);
	restore(ps);
	return OK;
}
