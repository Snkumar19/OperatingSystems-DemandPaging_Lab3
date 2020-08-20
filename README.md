# CSC501-DemandPaging_Lab3

Demand paging is a method of mapping a large address space into a relatively small amount of physical memory. It allows a program to use an address space that is larger than the physical memory, and access non-contiguous sections of the physical memory in a contiguous way. Demand paging is accomplished by using a “backing store” (usually disk) to hold pages of memory that are not currently in use.

The goal of this project is to implement the following system calls and their supporting infrastructure.

**System Calls**

- SYSCALL xmmap (int virtpage, bsd_t source, int npages):
Much like its Unix counterpart (see man mmap), it maps a source file (“backing store” here) of size npages pages to the virtual page virtpage. A process may call this multiple times to map data structures, code, etc.

- SYSCALL xmunmap (int virtpage):
This call, like munmap, should remove a virtual memory mapping. See man munmap for the details of the Unix call.

- SYSCALL vcreate (int *procaddr, int ssize, int hsize, int priority, char *name, int nargs, long args):
This call will create a new Xinu process. The difference from create() is that the process’ heap will be private and exist in its virtual memory.
The size of the heap (in number of pages) is specified by the user through hsize.

- create() should be left (mostly) unmodified. Processes created with create() should not have a private heap, but should still be able to use xmmap().

- WORD *vgetmem (int nbytes):
Much like getmem(), vgetmem() will allocate the desired amount of memory if possible. The difference is that vgetmem() will get the memory from a process’ private heap located in virtual memory. getmem() still allocates memory from the regular Xinu kernel heap.

- SYSCALL srpolicy (int policy):
This function will be used to set the page replacement policy to Second-Chance (SC) or Least Frequently Used (LFU). You can declare constant SC as 3 and LFU as 4 for this purpose.

- SYSCALL vfreemem (block_ptr, int size_in_bytes):
Implement a corresponding vfreemem() for vgetmem() call. vfreemem() takes two parameters and returns OK or SYSERR. The two parameters are similar to those of the original freemem() in Xinu. The type of the first parameter block_ptr depends on your own implementation.


**Backing Stores

- bsd_t is the type of backing store descriptors. Each descriptor is used to reference a backing store. Its type declaration is in h. This type is merely unsigned int. There are 16 backing stores. You will use IDs 0 through 15 to identify them.
- int get_bs (bsd_t store, unsigned int npages) requests a new backing store with ID store of size npages (in pages, not bytes). If a new backing store can be created, or a backing store with this ID already exists, the size of the new or existing backing store is returned. This size is in pages. If a size of 0 is requested, or the creation encounters an error, SYSERR should be returned. Also for practical reasons, npages should be no more than 128.
- int release_bs (bsd_t store) releases the backing store with the ID store.
- SYSCALL read_bs (char *dst, bsd_t store, int page) copies the page-th page from the backing store referenced by store to dst. It returns OK on success, SYSERR otherwise. The first page of a backing store is page zero.
- SYSCALL write_bs (char *src, bsd_t store, int page) copies a page referenced by src to the page-th page of the backing store referenced by store. It returns OK on success, SYSERR otherwise.

**Memory Layout

Xinu version compiles to about 100KB, or 25 pages. There is an area of memory from page 160 through the end of page 405 that cannot be used (this is referred to as the “HOLE” in initialize.c). We will place the free frames into pages 1024 through 4095, giving 3072 frames.

The frames will be used to store resident pages, page directories, and page tables. The remaining free memory below page 4096 is used for Xinu’s kernel heap (organized as a freelist). getmem()and getstk() will obtain memory from this area (from the bottom and top, respectively).

All memory below page 4096 will be global. That is, it is usable and visible by all processes and accessible by simply using actual physical addresses. As a result, the first four page tables for every process will be the same, and thus should be shared.

Memory at page 4096 and above constitute a process’ virtual memory. This address space is private and visible only to the process which owns it. Note that the process’ private heap and (optionally) stack are located somewhere in this area.

**Backing Store Emulation

Since our version of Xinu does not have file system support, we need to emulate the backing store with physical memory. A Xinu instance has 16 MB (4096 pages) of real memory in total. We reserve the top 8MB real memory as backing stores. We have 16 backing stores and each backing store maps up to 128 pages (each page is 4K size). 

**Page Tables and Page Directories

Page tables and page directories (i.e. outer page tables) can be placed in any free frames. As page tables are always resident in memory, it is not practical to allocate all potential page tables for a process when it is created . To map all 4 GB of memory would require 4 MB of page tables! To conserve memory, page tables must be created on-demand. That is, the first time a page is legally touched (i.e. it has been mapped by the process) for which no page table is present, a page table should be allocated. Conversely, when a page table is no longer needed it should be removed to conserve space.

**Page Replacement Policies

For SC, when a frame is allocated for a page, you insert the frame into a circular queue. When a page replacement occurs, SC first looks at the current position in the queue (current position starts from the head of the queue), checks to see whether its reference bit is set (i.e., pt_acc = 1). If it is not set, the page is swapped out. Otherwise, the reference bit is cleared, the current position moves to the next page and this process is repeated. If all the pages have their reference bits set, on the second encounter, the page will be swapped out, as it now has its reference bit cleared.

For LFU, the algorithm keeps track of the frame use history through a usage counter. Every time a frame is used – allocated or accessed (i.e. read or write) allocated or accessed for replacement, the usage counter of that frame increments. When there is not enough space for a newly allocated frame, the frame with the smallest usage counter value will be replaced. If there is a tie on the usage counter between two frames, the frame mapping to the larger virtual page number will be replaced.

 

