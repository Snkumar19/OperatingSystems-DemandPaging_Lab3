/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */

int pfCounter = 0;
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);
	
	
	//if (pfCounter > 27 )
	//	shutdown();
	unsigned long faulted_Address;
	virt_addr_t* faulted_vAddress;
  	unsigned int faulted_pd, faulted_pt, faulted_pg;
	// kprintf("To be implemented!\n");
	// Section 4.5.1 - Fage Faults
	
	// Section 4.5.1.1 -  4.5.1.3 Get the Faulted Address, vp be the virtual page number 
	// and pd be current page directory
	 
	faulted_Address = read_cr2();
	faulted_vAddress = (virt_addr_t*)&faulted_Address; //TypeDef into strcture
	
	faulted_pd = faulted_vAddress->pd_offset;	
	faulted_pt = faulted_vAddress->pt_offset;
	faulted_pg = faulted_vAddress->pg_offset;
	
	// Section 4.5.1.4 Check that faulted address is mapped onto PD, else kill the process 
	unsigned long pd = proctab[currpid].pdbr;
	
	pd_t* pde; pt_t* pte;

	//kprintf(" PageFault - %d at  %lu \n", ++pfCounter, faulted_Address);
	unsigned long baseAddress = pd + faulted_pd*sizeof(pd_t);
	pde = baseAddress;		
	if (pde->pd_pres == 0)
	{
			
		int pd_frm_offset  = pde->pd_base - FRAME0;
		frm_tab[pd_frm_offset].fr_refcnt++;
		//kprintf(" - Case 1 \t");
		int frmNumber = init_pt(currpid, 1); 
		if (frmNumber == SYSERR)
		{
			//kprintf ("PFINT ERROR: CASE-1,1 Killing Proc \n");
			kill(currpid);  
			restore(ps);
			return SYSERR;
		}	
		int return_val = allocate_page_directory (currpid, baseAddress, frmNumber, 1); 	
		if (return_val == SYSERR)
                {
			//kprintf ("PFINT ERROR: CASE-1,2 Killing Proc \n");
                        kill(currpid);
                        restore(ps);
                        return SYSERR;
                }
	
	}
	
	pte = (pt_t*)(pde->pd_base*NBPG + faulted_pt*sizeof(pt_t));
	if (pte->pt_pres == 0)
	{
		
		//kprintf(" - Case 2 \n");
		int avail = 0;
		if (get_frm(&avail) == SYSERR)
		{
			//kprintf ("PFINT ERROR: CASE-2,1 Killing Proc \n");
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
		bsd_t store = 0;
		int pageth = 0;
		if (bsm_lookup (currpid, faulted_Address, &store, &pageth) == SYSERR)
		{
			//kprintf ("PFINT ERROR: CASE-2,2 Killing Proc \n");
			kill(currpid);
                        restore(ps);
                        return SYSERR;
		}
		pte->pt_pres = 1;
		pte->pt_write = 1;
		pte->pt_base = FRAME0 + avail;	
		
		frm_tab[avail].fr_status = FRM_MAPPED;
		frm_tab[avail].fr_pid = currpid;
		frm_tab[avail].fr_vpno = faulted_Address>>12;	
		frm_tab[avail].fr_type = FR_PAGE;
		
		int pt_frm_offset  = pte->pt_base - FRAME0;
		frm_tab[pt_frm_offset].fr_refcnt++;
		//kprintf ("ptfrmoffset : %d \n", pt_frm_offset);
		frm_tab[avail].fr_refcnt++;
		read_bs((char*)(pte->pt_base<<12), store, pageth);	

		//Frame needs to be inserted as per Replacement Policy
		enqueue_FrametoSC(avail);
	}
	write_cr3(pd);
	
	restore(ps);
	return OK;
}


