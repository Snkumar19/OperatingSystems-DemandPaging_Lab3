/* initialize.c - nulluser, sizmem, sysinit */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

// Flag is 0 for Global Page Tables, 1 for Page Fault Call
int init_pt(int pid, int flag)
{
	int i = 0, j = 0, avail = 0;
	pt_t *pte;
		
	/* For the First 4 entries in the page */
	if (flag == 0) {
		for ( i = 0; i < 4; i++)
		{
			// Get a free frame
			if (get_frm(&avail) == SYSERR)
				return SYSERR;
			//kprintf("\n INIT - Global PDT : AVAIL = %d", avail);
			//kprintf("\n AVAIL ADDRESS= %x", &avail);
			frm_tab[avail].fr_status = FRM_MAPPED;
			frm_tab[avail].fr_pid = pid;
			frm_tab[avail].fr_type = FR_TBL;
			// Calc. Location for PTE
			pte = (FRAME0 + avail)*NBPG; 
			for (j = 0; j < NFRAMES; j++)
			{
				  pte->pt_pres   = 1;           		/* page is present?             */
				  pte->pt_write  = 1;           		/* page is writable?            */
				  pte->pt_user   = 0;           		/* is use level protection?     */
				  pte->pt_pwt    = 0;           		/* write through for this page? */
				  pte->pt_pcd    = 0;           		/* cache disable for this page? */
				  pte->pt_acc    = 0;           		/* page was accessed?           */
				  pte->pt_dirty  = 0;           		/* page was written?            */
				  pte->pt_mbz    = 0;           		/* must be zero                 */
				  pte->pt_global = 1;           		/* should be zero in 586        */
				  pte->pt_avail  = 0;           		/* for programmer's use         */
				  pte->pt_base   = i*FRAME0 + j;           	/* location of page?            */
				  //if (j == 0 || j == NFRAMES-1)
				  //	kprintf("\n %d PTE_BASE= %d", j, pte->pt_base);
				  pte++;
			}
		}
		return OK;
	}

	if (flag == 1) 
	{
		if (get_frm(&avail) == SYSERR)
		{	
			return SYSERR;
		}
		// Calc. Location for PTE
		frm_tab[avail].fr_status = FRM_MAPPED;
                frm_tab[avail].fr_pid = pid;
                frm_tab[avail].fr_type = FR_TBL;
		frm_tab[avail].fr_refcnt++;	
		pte = (FRAME0 + avail)*NBPG; 
		for (j = 0; j < NFRAMES; j++)
		{
			  pte->pt_pres   = 0;           		/* page is present?             */
			  pte->pt_write  = 0;           		/* page is writable?            */
			  pte->pt_user   = 0;           		/* is use level protection?     */
			  pte->pt_pwt    = 0;           		/* write through for this page? */
			  pte->pt_pcd    = 0;           		/* cache disable for this page? */
			  pte->pt_acc    = 0;           		/* page was accessed?           */
			  pte->pt_dirty  = 0;           		/* page was written?            */
			  pte->pt_mbz    = 0;           		/* must be zero                 */
			  pte->pt_global = 0;           		/* should be zero in 586        */
			  pte->pt_avail  = 0;           		/* for programmer's use         */
			  pte->pt_base   = 0;           		/* location of page?            */
			  pte++;
		}
		return avail;
	}
	return SYSERR;
}

