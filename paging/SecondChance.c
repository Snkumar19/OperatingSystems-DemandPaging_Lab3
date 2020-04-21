/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <proc.h>

/*------------------------------------------------------------------------
 * Queue Functions for insertion and deletion
 * ----------------------------------------------------------------------
*/

int isEmptyQueue()
{
	if (SCQHead == EMPTY)
		return OK;
	return SYSERR;
}

int ifQueueNotFull()
{
	if((SCQHead == 0 && SCQTail == NFRAMES - 1) || (SCQHead == SCQTail + 1))
        {
                return SYSERR;
        }
	return OK;
	
}
/*-------------------------------------------------------------------------
 * init_SC - initialize the circular queue for SC
 *-------------------------------------------------------------------------
*/
SYSCALL init_SC()
{
	//  Initialize Circular Queue 
	//kprintf("To be implemented!\n");
	int i = 0;
	for ( i = 0; i < NFRAMES; i++) {
		CircularQueue[i] = EMPTY;
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * enqueue_FraneToSC - Insert the Frame to the Head ciruclar queue
 *-------------------------------------------------------------------------
 */
SYSCALL enqueue_FrametoSC(int frame)
{
	//kprintf("To be implemented!\n");
	if(ifQueueNotFull() != OK)
        {
                kprintf("Circular Queue Overflow \n");
                return SYSERR;
        }
        if (isEmptyQueue() == OK)  /*If queue is empty */
        {
                SCQHead = 0;
                SCQTail = 0;
        }
        else
        {
                if(SCQTail == NFRAMES-1)        /*SCQTail is at last position of queue */
                        SCQTail = 0;
                else
                        SCQTail = SCQTail+1;
        }
        CircularQueue[SCQTail] = frame ;
	
	return OK;
}

/*-------------------------------------------------------------------------
 * dequeue_FraneToSC - Return First Inserted Frame from  ciruclar queue
 *-------------------------------------------------------------------------
 */

SYSCALL dequeue_FrameToSC() 
{
	if (isEmptyQueue() == OK)
        {
                kprintf("Circular Queue Underflow\n");
                return SYSERR;
        }
        int return_val = CircularQueue[SCQHead];
        
	if(SCQHead == SCQTail) /* queue has only one element */
        {
                SCQHead = -1;
                SCQTail=-1;
        }
        else
        {
                if(SCQHead == NFRAMES-1)
                        SCQHead = 0;
                else
                        SCQHead = SCQHead+1;
        }


	return return_val;

}
/*-------------------------------------------------------------------------
 * Print Circular Queue - Debug Print only 
 *-------------------------------------------------------------------------
 */

void PrintCirularQueue() 
{
	int SCQHead_pos = SCQHead, SCQTail_pos = SCQTail;
        if(isEmptyQueue() == OK)
        {
                kprintf("Queue is empty\n");
                return;
        }
        kprintf("Queue elements are :\n");
        if( SCQHead_pos <= SCQTail_pos )
                while(SCQHead_pos <= SCQTail_pos)
                {
                        kprintf("%d ",CircularQueue[SCQHead_pos]);
                        SCQHead_pos++;
			if ( SCQHead_pos % 9 == 0)
				kprintf("\n");
                }
        else
        {
                while(SCQHead_pos <= NFRAMES-1)
                {
                        kprintf("%d ",CircularQueue[SCQHead_pos]);
                        SCQHead_pos++;
			if ( SCQHead_pos % 9 == 0)
                                kprintf("\n");

                }
                SCQHead_pos = 0;
                while(SCQHead_pos <= SCQTail_pos)
                {
                        kprintf("%d ",CircularQueue[SCQHead_pos]);
                        SCQHead_pos++;
			if ( SCQHead_pos % 9 == 0)
                                kprintf("\n");

                }
        }
        kprintf("\n Head is at %d and Tail is %d \n", SCQHead, SCQTail);

}



/*-------------------------------------------------------------------------
 * Get frame from Second Chance Queuee - Policy implementation 
 *-------------------------------------------------------------------------
 */
SYSCALL get_FrameUsingSC()
{

  	//kprintf("Second Chance!\n");
	//PrintCirularQueue();
  	int SCQHead_pos = SCQHead, SCQTail_pos = SCQTail, return_val = -1;
        if(isEmptyQueue() == OK)
        {
                kprintf("Queue is empty\n");
                return SYSERR;
        }
	
	int loopVar = 0;
	
	/* Since Ciruclar Array is array based, if Tail and Head markers might interchange places */
	/* Use Loop markers to see how to iterate the loop  */
	if( SCQHead_pos <= SCQTail_pos )
		loopVar = 1;	
	
	if (loopVar == 1) 
	{
		/* Since control comes here after there are no UNMAPEED Entries in FRM_TAB, start from beginning */
		int current_frame = CircularQueue[SCQHead_pos];
		pt_t *pte; pd_t *pde;
		virt_addr_t *virtualAddress; 
		unsigned long temp;
		unsigned int pd_offset = 0, pt_offset = 0;
		int frameObtained  = 0;
		while (!frameObtained)
		{
			temp  = frm_tab[current_frame].fr_vpno;
			virtualAddress = (virt_addr_t*)&temp;
			unsigned int pd_offset = virtualAddress->pd_offset;
			unsigned int pt_offset = virtualAddress->pt_offset;
			pde =  proctab[currpid].pdbr + pd_offset*sizeof(pd_t);
			pte = (pt_t*)(pde->pd_base << 12 + pt_offset*sizeof(pt_t));
			/*  Check the accessed Bit and decide which one to remove */
			if (pte->pt_acc == 1)
				pte->pt_acc = 0;		
			else
			{
				/* Delete element from array and return the frame number */	
				int i ;
				return_val = CircularQueue[SCQHead_pos];
				delete_SCEntryFromQueue(SCQHead_pos);
				frameObtained = 1;
				SCQTail--;
				if (policy_DEBUG)
					kprintf ("Second Chance Replaced Frame : %d\n", return_val);
				break;
			}
			/* Go to the next element */	
			SCQHead_pos++;
			if (SCQHead_pos >  SCQTail_pos)	
				SCQHead_pos = SCQHead;
			
			current_frame = CircularQueue[SCQHead_pos];
		}
	}

	else
	   	kprintf ("SC - Hitting Else case \n");	
	
	//PrintCirularQueue();

	return return_val;
}


void delete_SCEntryFromQueue(int pos)
{
	int i = 0;
	for ( i = pos+1 ; i <= SCQTail ; i++)
        	CircularQueue[i-1] =  CircularQueue[i];

}

int findFrameinQueue(int frameNo)
{
	int i = 0;
	for ( i = 0; i < NFRAMES; i++)
	{
		if (frameNo == CircularQueue[i])
			return i;

	}
	return SYSERR;
}
