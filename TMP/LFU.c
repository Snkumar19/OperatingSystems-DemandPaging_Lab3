/* LFU.c - manage LFU */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


SYSCALL get_FrameUsingLFU() {

	int i = 0, lowest_refCnt = MAXINT, frameCorrespondingToLowest = SYSERR;
	for ( i = 0 ; i < NFRAMES ; i++) {
		if (frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE) {
			if (frm_tab[i].fr_refcnt < lowest_refCnt) 
			{
				lowest_refCnt = frm_tab[i].fr_refcnt ;
				frameCorrespondingToLowest = i;
			}			

		}
	}
	if (policy_DEBUG)
		kprintf("LFU is replacing Frame: %d\n",frameCorrespondingToLowest);	
	return frameCorrespondingToLowest;
}
