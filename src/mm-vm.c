/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *\
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, addr_t vicfpn , addr_t swpfpn)
{
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    return 0;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, addr_t vmastart, addr_t vmaend)
{
  struct vm_area_struct *vma = caller->mm->mmap;
  if (vma == NULL)
  {
    return -1;
  }

  /* TODO validate the planned memory area is not overlapped */
  /* Iterate through existing VMAs to check for overlap */
  while (vma != NULL)
  {
    /* Skip the current VMA being modified (if any) */
    if (vma->vm_id != vmaid) {
        /* Check if ranges overlap: (StartA < EndB) and (StartB < EndA) */
        if ((vma->vm_start < vmaend) && (vmastart < vma->vm_end)) {
            return -1; // Overlap detected
        }
    }
    vma = vma->vm_next;
  }

  /* End TODO*/

  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *\
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = (int)inc_sz;
  int incnumpage =  0;
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  int old_end = cur_vma->vm_end;

  /* Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0) {
    free(newrg);
    return -1; /* Overlap detected */
  }

  /* TODO: The obtained vm area (area) is used to create a new region (newrg) 
     and map it to physical RAM */
  
  newrg->rg_start = area->rg_start;
  newrg->rg_end = area->rg_end;
  newrg->rg_next = NULL;

  /* Calculate number of pages needed.
     PAGING_PAGE_ALIGNSZ rounds up to the nearest multiple of PAGING_PAGESZ */
  inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz); 
  incnumpage = inc_amt / PAGING_PAGESZ;

  /* Map the region to RAM */
  if (vm_map_ram(caller, newrg->rg_start, newrg->rg_end, 
                  old_end, incnumpage, newrg) < 0)
  {
    free(newrg);
    return -1; /* Mapping failed (e.g., Out of Memory) */
  }
  
  /* Update VMA Limit */
  cur_vma->vm_end += inc_amt;
  cur_vma->sbrk += inc_sz; /* sbrk tracks actual usage, not aligned usage */

  /* Free the temporary area structure if it was dynamically allocated inside get_vm_area...
     (Depending on implementation of get_vm_area_node_at_brk, usually it returns a pointer to static or malloc) 
     Assuming we need to keep newrg for the list, but 'area' might be temp. 
     However, vm_map_ram usually enlists newrg. */

  return 0;
}

/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the victim page finding strategy */
  /* Simple strategy: Scan the Page Directory to find a present page.
     (In a real FIFO implementation, we would maintain a queue).
     Since this assignment often implies a simple Round-Robin/FIFO on the PGD 
     or using the fifo_pgn list if available. */

  if (pg != NULL) {
      /* Pick the last one in the list (or first, depending on list implementation) */
      /* Here we assume the head of fifo_pgn list is the victim */
      *retpgn = pg->pgn;
      
      /* Move head to next (removing head is done by caller or we do it here?)
         Usually finding doesn't remove. Removal happens at swap time. */
      return 0;
  }

  /* If fifo list is not maintained, scan PGD for any valid page */
  for (int i = 0; i < PAGING_MAX_PGN; i++) {
      if (mm->pgd[i] != 0) { // Check if PTE is not empty
          /* Check if Present bit is set */
          if (PAGING_PTE_PAGE_PRESENT(mm->pgd[i])) {
              *retpgn = i;
              return 0;
          }
      }
  }

  return -1;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size 
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;

  /* Probe mechanically ranges to find the fit */
  while (rgit != NULL)
  {
    if (rgit->rg_start + size <= rgit->rg_end)
    { /* Current region has enough space */
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;

      /* Update left space in free region */
      if (rgit->rg_start + size < rgit->rg_end)
      {
        rgit->rg_start = rgit->rg_start + size;
      }
      else
      { /* Exact match, remove this free region node */
        struct vm_rg_struct *nextrg = rgit->rg_next;

        /* We need to find the previous node to unlink, 
           but this is a singly linked list. 
           So usually we copy content or handle head specially. 
           For simplicity in this skeleton: */
        if (cur_vma->vm_freerg_list == rgit) {
             cur_vma->vm_freerg_list = nextrg;
        } else {
             /* Traverse again to find prev (inefficient but safe) */
             struct vm_rg_struct *prev = cur_vma->vm_freerg_list;
             while(prev->rg_next != rgit) prev = prev->rg_next;
             prev->rg_next = nextrg;
        }
        free(rgit);
      }
      return 0;
    }
    rgit = rgit->rg_next;
  }

  return -1; // No suitable free region found
}

//#endif