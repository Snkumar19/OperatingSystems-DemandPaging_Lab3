/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */

//struct fr_map_t frm_tab[NFRAMES];

//fr_map_t frm_tab[NFRAMES];
SYSCALL init_frm()
{
	// kprintf("To be implemented!\n");
	//kprintf("\n Frame Init \n");
	STATWORD ps;
	disable(ps);
	/* Initialize Circular Queue */
	
	if (init_SC() != OK)
	{
		restore(ps);
		return SYSERR;
	}
	
	int i = 0;
	for( i = 0; i < NFRAMES; i++)
	{
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;
	}
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  	STATWORD ps;
	disable(ps);
	//kprintf("\nGet FRM !");
  	int i = 0;
	for ( i = 0; i < NFRAMES; i++)
	{
		if (frm_tab[i].fr_status == FRM_UNMAPPED )
		{
			*avail = i;
			//kprintf ("\n Return - %d \n" , i);
			return OK;
		}
	}
	/* If there are no unmapeed frames then pick a frame as per replacement policy */
	
	int return_val = SYSERR;
	if ( page_replace_policy == SC ) 
		return_val = get_FrameUsingSC(); 
	else
		return_val = get_FrameUsingLFU();
	
	if (return_val != SYSERR) 
	{
		*avail = return_val;
		restore(ps);
		return OK;
	}
	
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD ps;
	disable(ps);
  	
	//kprintf("To be implemented!\n");
  	
	pd_t *pde; pt_t *pte;
		
	// When trying to free a Frame, eviction will depend on which type of Frame it is
	
	if (frm_tab[i].fr_status != FRM_UNMAPPED)
	{	
		kprintf ("\n ERROR: Evicting UNMAPPED Frame.. \n");
                restore(ps);
                return SYSERR;


	}	
	if (frm_tab[i].fr_type != FR_PAGE || i < 0)
	{	
		if (frm_tab[i].fr_type != FR_PAGE)
			kprintf ("\n ERROR: Evicting Frame for PD or PT is illegal.. \n");
		else 
			kprintf ("\n ERROR: Evicting illegal Frame.. \n");
		restore(ps);	
  		return SYSERR;
	}


	unsigned long temp = frm_tab[i].fr_vpno << 12;
	virt_addr_t* vaddr = (virt_addr_t*)&temp;	
	pde = proctab[frm_tab[i].fr_pid].pdbr + (vaddr->pd_offset)*sizeof(pd_t);
	pte = pde->pd_base<<12 + (vaddr->pt_offset*sizeof(pt_t));
	
	bsd_t bs_id; int pageth; 
	
	if (bsm_lookup (frm_tab[i].fr_pid, vaddr, &bs_id, &pageth) != SYSERR)
		write_bs( (i+ FRAME0) << 12, bs_id, pageth ) ;
	else
	{
		kprintf ("\n ERROR: Evicting Frame - illegal BSM MAP.. \n");
		restore(ps);
                return SYSERR;
	}

	clear_frame(i);
	pte->pt_base = 0;
	pte->pt_pres  = 0;
	

	int pd_frame = pde->pd_base - FRAME0;
	
	frm_tab[pd_frame].fr_refcnt--;
	if (frm_tab[pd_frame].fr_refcnt == 0)
	{	
		clear_frame(pd_frame);
		pde->pd_pres  = 0;            /* page table present?          */
                pde->pd_base  = 0;           /* location of page table?      */

	}
	restore(ps);
	return OK;
}


SYSCALL clear_frame( int frame )
{

		frm_tab[frame].fr_status = FRM_UNMAPPED;
		frm_tab[frame].fr_pid = -1;
		frm_tab[frame].fr_vpno = 0;
		frm_tab[frame].fr_refcnt = 0;
		frm_tab[frame].fr_type = FR_PAGE;
		frm_tab[frame].fr_dirty = 0;

		return OK;
}
