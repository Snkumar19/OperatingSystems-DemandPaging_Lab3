// Adding file to allocate Page Directory
// Since this needs to be called from multiple locations
 
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int allocate_page_directory (int pid, int baseAddress, int Offset, int flag)
{

	//kprintf ("To be Implemented \n");
	//
	int i = 0, avail = 0;
        pd_t *pde;
	if (flag == 0)
	{
		if (get_frm(&avail) == SYSERR)
		{
			return SYSERR;	
		}
	// If you get a frame, then do frame tab manipulation 
		frm_tab[avail].fr_status = FRM_MAPPED;
		frm_tab[avail].fr_pid = pid;
		frm_tab[avail].fr_vpno = 0;
		frm_tab[avail].fr_refcnt = 1;
		frm_tab[avail].fr_type = FR_DIR;
		frm_tab[avail].fr_dirty = 0;

		// Set Proc Tab registers and Initialize pde[i].
		unsigned long bAddress = (avail + FRAME0)*NBPG ;
		proctab[pid].pdbr = bAddress;
		pde = bAddress; 
		
		//kprintf("\n Allocate Page Directory : AVAIL = %d, pde = %lu", avail, proctab[pid].pdbr) ;
		for ( i = 0; i<NFRAMES; i++)
		{
			
			  // Make the first 4 entries present and update PD_BASE for those 4
			  
			  if ( i < 4) {
				pde[i].pd_pres  = 1;            /* page table present?          */
				pde[i].pd_base  = FRAME0 + i;   /* location of page table?      */
			  }
			  else {
				pde[i].pd_pres  = 0;            /* page table present?          */
				pde[i].pd_base  = 0;           /* location of page table?      */
			  }
			  
			  pde[i].pd_write = 1;            /* page is writable?            */
			  pde[i].pd_user  = 0;            /* is use level protection?     */
			  pde[i].pd_pwt   = 0;            /* write through cachine for pt?*/
			  pde[i].pd_pcd   = 0;            /* cache disable for this pt?   */
			  pde[i].pd_acc   = 0;            /* page table was accessed?     */
			  pde[i].pd_mbz   = 0;            /* must be zero                 */
			  pde[i].pd_fmb   = 0;            /* four MB pages?               */
			  pde[i].pd_global= 0;            /* global (ignored)             */
			  pde[i].pd_avail = 0;            /* for programmer's use         */
		}

		return OK;
	}

	if (flag == 1)
	{
		pde = baseAddress;
		pde->pd_pres  = 1;            		/* page table present?          */
                pde->pd_base  = FRAME0 + Offset;   	/* location of page table?      */

	  	pde[i].pd_write = 1;            /* page is writable?            */
	  	pde[i].pd_user  = 0;            /* is use level protection?     */
	  	pde[i].pd_pwt   = 0;            /* write through cachine for pt?*/
	  	pde[i].pd_pcd   = 0;            /* cache disable for this pt?   */
	  	pde[i].pd_acc   = 0;            /* page table was accessed?     */
	  	pde[i].pd_mbz   = 0;            /* must be zero                 */
	  	pde[i].pd_fmb   = 0;            /* four MB pages?               */
	  	pde[i].pd_global= 0;            /* global (ignored)             */
	  	pde[i].pd_avail = 0;            /* for programmer's use         */
		
                return OK;

	}

  return SYSERR;
		
}

