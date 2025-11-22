#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

static BYTE _ram[RAM_SIZE];

static struct {
	uint32_t proc;	// ID of process currently uses this page
	int index;	// Index of the page in the list of pages allocated
			// to the process.
	int next;	// The next page in the list. -1 if it is the last
			// page.
} _mem_stat [NUM_PAGES];

static pthread_mutex_t mem_lock;

void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

/* get offset of the virtual address */
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

/* get the first layer index */
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + PAGE_LEN);
}

/* get the second layer index */
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Search for page table table from the a segment table */
static struct trans_table_t * get_trans_table(
		addr_t index, 	// Segment level index
		struct page_table_t * page_table, // first level table
		struct pcb_t * proc) {
	return NULL;
}

/* Translate virtual address to physical address. If [virtual_addr] is valid,
 * return 1 and write its physical counterpart to [physical_addr].
 * Otherwise, return 0 */
int translate(addr_t virtual_addr, addr_t * physical_addr, struct pcb_t * proc) {
	/* Determine physical address from virtual address [virtual_addr] */
	
	/* Check if the process has a valid memory management structure */
	if (proc == NULL || proc->mm == NULL || proc->mm->pgd == NULL) {
		return 0; 
	}

	/* Get Page Number (PGN) and Offset from Virtual Address */
	/* Assuming macros are defined in mm.h (included via mem.h or implicitly) */
	/* PGN extraction logic typically: virtual_addr >> PAGING_ADDR_FPN_LOBIT */
	
	/* Note: Based on mm.c implementation, we use Single-Level Paging */
	addr_t pgn = PAGING_PGN(virtual_addr);
	addr_t offset = PAGING_OFFST(virtual_addr);

	/* Check bounds if necessary */
	if (pgn >= PAGING_MAX_PGN) {
		return 0;
	}

	/* Lookup the Page Table Entry (PTE) in the process's Page Directory */
	addr_t pte = proc->mm->pgd[pgn];

	/* Check if the page is PRESENT */
	if (PAGING_PTE_PAGE_PRESENT(pte)) {
		/* Extract Frame Page Number (FPN) from PTE */
		addr_t fpn = PAGING_PTE_FPN(pte);
		
		/* Calculate Physical Address: (FPN * PageSize) + Offset */
		*physical_addr = (fpn * PAGING_PAGESZ) + offset;
		
		return 1; // Success
	}

	return 0; // Page fault or Invalid address
}

int alloc_mem(int num_pages, struct pcb_t * proc) {
	int i, j;
	int ret_mem = 0;
	int num_allocated = 0;
	int previous_page = -1;

	pthread_mutex_lock(&mem_lock);

	/* Find [num_pages] free frames in _mem_stat */
	int free_pages_found = 0;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc == 0) {
			free_pages_found++;
		}
	}

	/* If enough memory is available */
	if (free_pages_found >= num_pages && mem_avail) { // Assuming mem_avail is global or macro
		/* Allocate new memory region to the process (Virtual Address) */
		ret_mem = proc->bp;
		proc->bp += num_pages * PAGE_SIZE; // Increase Break Pointer

		/* Update status of physical pages in _mem_stat */
		for (i = 0; i < NUM_PAGES && num_allocated < num_pages; i++) {
			if (_mem_stat[i].proc == 0) {
				/* Update entry */
				_mem_stat[i].proc = proc->pid;
				_mem_stat[i].index = num_allocated;
				
				/* Link the pages if necessary (logic for tracking list) */
				if (previous_page != -1) {
					_mem_stat[previous_page].next = i;
				}
				previous_page = i;
				_mem_stat[i].next = -1; // Currently last page

				num_allocated++;
			}
		}
	} else {
		ret_mem = 0; // Failed to allocate
	}

	pthread_mutex_unlock(&mem_lock);
	return ret_mem;
}

int free_mem(addr_t address, struct pcb_t * proc) {
	/* DO NOTHING HERE. This mem is obsoleted */
	return 0;
}

int read_mem(addr_t address, struct pcb_t * proc, BYTE * data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		return 0;
	}else{
		return 1;
	}
}

int write_mem(addr_t address, struct pcb_t * proc, BYTE data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		return 0;
	}else{
		return 1;
	}
}

void dump(void) {
	int i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
				i << OFFSET_LEN,
				((i + 1) << OFFSET_LEN) - 1,
				_mem_stat[i].proc,
				_mem_stat[i].index,
				_mem_stat[i].next
			);
			int j;
			for (j = i << OFFSET_LEN;
				j < ((i+1) << OFFSET_LEN) - 1;
				j++) {
				
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
					
			}
		}
	}
}