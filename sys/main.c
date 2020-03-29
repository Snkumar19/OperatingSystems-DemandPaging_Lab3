/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
extern int page_replace_policy;

int main()
{
	int ret;
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
	kprintf("\n Default policy = %d",grpolicy());
	
	ret = srpolicy(LFU);
	kprintf("\n Set policy LFU = %d, ret = %d",grpolicy(), ret);
	ret = srpolicy(100);
	kprintf("\n Test SYSERR = %d",ret);
        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call terminates the QEMU process.
         */
        shutdown();
}
