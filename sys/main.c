#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

extern void PrintCirularQueue();
#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1


void printBSM(bsd_t i)
{
	kprintf (" \n BSM - Details \n");
	kprintf ("  BSM Status 		- %d \n", bsm_tab[i].bs_status);
	kprintf ("  BSM PID 		- %d \n", bsm_tab[i].bs_pid);
	kprintf ("  BSM VPNO 		- %d \n", bsm_tab[i].bs_vpno);
	kprintf ("  BSM NPAGES		- %d \n", bsm_tab[i].bs_npages);
	kprintf ("  BSM SharedProcCnt  	- %d \n", bsm_tab[i].bs_sharedProcCnt);
	if ( bsm_tab[i].bs_sharedProcCnt != 0)
	{
		int j = 0;
		for ( j = 0; j < bsm_tab[i].bs_sharedProcCnt; j++)
		{
			kprintf ("  BSM SharedPID   - %d \n", bsm_tab[i].bs_sharedPID[j]);
			kprintf ("  BSM SharedVPNO   - %d \n", bsm_tab[i].bs_sharedVPNO[j]);
			kprintf ("  BSM SharedNPages   - %d \n", bsm_tab[i].bs_sharedNPages[j]);
		}
	}
}
void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;
	
	srpolicy(LFU);	
	kprintf ("Acquired BS - %d \n",  get_bs(TEST1_BS, 100));
	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}
	kprintf ("XMMAP Successful \n");
	
	printBSM(TEST1_BS);
	addr = (char*) PROC1_VADDR;
	
	for (i = 0; i < NFRAMES-7 ; i++) {
		*(addr + i * NBPG) = 'A' + (i%26);
		
	}
	sleep(3);

	for (i = 0; i < 26 ; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}
	kprintf ("XMUNMAP Called \n");
	xmunmap(PROC1_VPNO);
	kprintf ("XMUNMAP Complete \n");
	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;
	kprintf("Here at the Waall\n");
	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	x = vgetmem(1024);
        kprintf("heap allocated at %x\n", x);
        *x = 100;
        *(x + 1) = 200;
        kprintf("Here at the Waall\n");
        kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
	vfreemem(x, 1024);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 10; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 10; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

int main() {
	//page_replace_policy = LFU;
	int pid1;
	int pid2;
	
	kprintf ("\n Main Function ");
	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);
	
		
	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);
		
		
	kprintf("\n3: Frame test\n");
	pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	resume(pid1);
	sleep(3);
	
	//kprintf ("Printing Circular Queue \n");
	//PrintCirularQueue();
	
}
