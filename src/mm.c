/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */
 
 /* NOTICE this moudle is deprecated in LamiaAtrium release
  * the structure is maintained for future 64bit-32bit
  * backward compatible feature or PAE feature  
  */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

#if !defined(MM64)
/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

/*
 * init_pte - Initialize PTE entry
 */
int init_pte(addr_t *pte,
             int pre,    // present
             addr_t fpn,    // FPN
             int drt,    // dirty
             int swp,    // swap
             int swptyp, // swap type
             addr_t swpoff) // swap offset
{
  if (pre != 0) {
    if (swp == 0) { // Non swap ~ page online
      if (fpn == 0)
        return -1;  // Invalid setting

      /* Valid setting with FPN */
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    }
    else
    { // page swapped
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
      SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
    }
  }

  return 0;
}

/*
 * get_pd_from_address - Parse address to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table 
 */
int get_pd_from_address(addr_t addr, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
  /* For Single-Level Paging, this hierarchical parsing is not used. 
     Keeping it as deprecated/compatible stub. */
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 * get_pd_from_pagenum - Parse page number to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table 
 */
int get_pd_from_pagenum(addr_t pgn, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
  /* For Single-Level Paging, this hierarchical parsing is not used. 
     Keeping it as deprecated/compatible stub. */
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 * pte_set_swap - Set PTE entry for swapped page
 * @pte    : target page table entry (PTE)
 * @swptyp : swap type
 * @swpoff : swap offset
 */
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff)
{
  struct krnl_t *krnl = caller->krnl;
  addr_t *pte = &krnl->mm->pgd[pgn];
	
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for on-line page
 * @pte   : target page table entry (PTE)
 * @fpn   : frame page number (FPN)
 */
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn)
{
  struct krnl_t *krnl = caller->krnl;
  addr_t *pte = &krnl->mm->pgd[pgn];

  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

  return 0;
}


/* Get PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
uint32_t pte_get_entry(struct pcb_t *caller, addr_t pgn)
{
  struct krnl_t *krnl = caller->krnl;
  if (krnl == NULL || krnl->mm == NULL || krnl->mm->pgd == NULL) return 0;
  
  /* Access PGD directly for single-level paging */
  return krnl->mm->pgd[pgn];
}

/* Set PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
int pte_set_entry(struct pcb_t *caller, addr_t pgn, uint32_t pte_val)
{
	struct krnl_t *krnl = caller->krnl;
	krnl->mm->pgd[pgn]=pte_val;
	
	return 0;
}

/*
 * vmap_pgd_memset - map a range of page at aligned address
 */
int vmap_pgd_memset(struct pcb_t *caller,           // process call
                    addr_t addr,                       // start address which is aligned to pagesz
                    int pgnum)                      // num of mapping page
{
  /* This function is often used to clear or initialize PGD entries */
  /* We can leave it as 0 or implement memset if needed */
  return 0;
}

/*
 * vmap_page_range - map a range of page at aligned address
 */
addr_t vmap_page_range(struct pcb_t *caller,           // process call
                    addr_t addr,                       // start address which is aligned to pagesz
                    int pgnum,                      // num of mapping page
                    struct framephy_struct *frames, // list of the mapped frames
                    struct vm_rg_struct *ret_rg)    // return mapped region, the real mapped fp
{                                                   // no guarantee all given pages are mapped
  struct framephy_struct *fpit = frames;
  int  pgn = PAGING_PGN(addr);

  /* Map the allocated physical frames to the page table */
  for (int i = 0; i < pgnum; i++) { 
      if (fpit == NULL) return -1; // Not enough frames allocated

      /* Update the Page Table Entry (PTE) */
      // Use the provided pte_set_fpn function
      pte_set_fpn(caller, pgn + i, fpit->fpn);
      
      /* Move to next physical frame */
      fpit = fpit->fp_next;
  }

  return 0;
}

/*
 * alloc_pages_range - allocate req_pgnum of frame in ram
 * @caller    : caller
 * @req_pgnum : request page num
 * @frm_lst   : frame list
 */

addr_t alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
  int pgit, fpn;
  struct framephy_struct *newfp_str;

  /* Allocate frames from the Physical Memory (RAM) */
  for(pgit = 0; pgit < req_pgnum; pgit++) {
      
      // Use MEMPHY_get_freefp to get a free frame from RAM
      if(MEMPHY_get_freefp(caller->mram, &fpn) == 0) {
          newfp_str = malloc(sizeof(struct framephy_struct));
          newfp_str->fpn = fpn;
          newfp_str->owner = caller->mm;
          
          /* Add the new frame to the head of the list (LIFO) */
          newfp_str->fp_next = *frm_lst;
          *frm_lst = newfp_str;
      } else {
          // Out of memory: handle cleanup if necessary, or return error
          return -1;
      }
  }

  return 0;
}

/*
 * vm_map_ram - do the mapping all vm are to ram storage device
 * @caller    : caller
 * @astart    : vm area start
 * @aend      : vm area end
 * @mapstart  : start mapping point
 * @incpgnum  : number of mapped page
 * @ret_rg    : returned region
 */
addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, addr_t mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  struct framephy_struct *frm_lst = NULL;
  int ret_alloc;

  /* 1. Initialize the region info */
  ret_rg->rg_start = astart;
  ret_rg->rg_end = aend;
  ret_rg->rg_next = NULL;

  /* 2. Add the region to the process's memory map list */
  enlist_vm_rg_node(&caller->mm->mmap, ret_rg);

  /* 3. Allocate physical frames */
  ret_alloc = alloc_pages_range(caller, incpgnum, &frm_lst);
  if (ret_alloc < 0) return -1;

  /* 4. Map the virtual pages to the physical frames in the Page Table */
  vmap_page_range(caller, mapstart, incpgnum, frm_lst, ret_rg);

  return 0;
}

/* Swap copy content page from source frame to destination frame
 * @mpsrc  : source memphy
 * @srcfpn : source physical page number (FPN)
 * @mpdst  : destination memphy
 * @dstfpn : destination physical page number (FPN)
 **/
int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                   struct memphy_struct *mpdst, addr_t dstfpn)
{
  int i;
  BYTE data;
  addr_t srcaddr = srcfpn * PAGING_PAGESZ;
  addr_t dstaddr = dstfpn * PAGING_PAGESZ;

  /* Copy data byte by byte from source frame to destination frame */
  for (i = 0; i < PAGING_PAGESZ; i++) {
      // Read from source
      if (MEMPHY_read(mpsrc, srcaddr + i, &data) == 0) {
          // Write to destination
          MEMPHY_write(mpdst, dstaddr + i, data);
      }
  }
  return 0;
}

/*
 *Initialize a empty Memory Management instance
 * @mm:     self mm
 * @caller: mm owner
 */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  /* Allocate the Page Directory (Page Table) */
  // Single-level paging: size covers the entire virtual address space index
  mm->pgd = malloc(PAGING_MAX_PGN * sizeof(addr_t));

  /* Initialize all entries to 0 (invalid) */
  for (int i = 0; i < PAGING_MAX_PGN; i++) {
    mm->pgd[i] = 0;
  }

  /* Initialize list of regions to NULL */
  mm->mmap = NULL;

  /* Assign this mm struct to the caller process */
  caller->mm = mm;

  return 0;
}

struct vm_rg_struct *init_vm_rg(addr_t rg_start, addr_t rg_end)
{
  struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));

  rgnode->rg_start = rg_start;
  rgnode->rg_end = rg_end;
  rgnode->rg_next = NULL;

  return rgnode;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
  /* Insert the new node at the beginning of the list */
  rgnode->rg_next = *rglist;
  *rglist = rgnode;

  return 0;
}

int enlist_pgn_node(struct pgn_t **plist, addr_t pgn)
{
  struct pgn_t *pnode = malloc(sizeof(struct pgn_t));

  pnode->pgn = pgn;
  pnode->pg_next = *plist;
  *plist = pnode;

  return 0;
}

int print_list_fp(struct framephy_struct *ifp)
{
  struct framephy_struct *fp = ifp;
 
  printf("print_list_fp: ");
  if (fp == NULL) { printf("NULL list\n"); return -1; }
  printf("\n");
  while (fp != NULL )
  {
      printf("fp[%d]\n",fp->fpn);
      fp = fp->fp_next;
  }
  printf("\n");
  return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
  struct vm_rg_struct *rg = irg;
 
  printf("print_list_rg: ");
  if (rg == NULL) { printf("NULL list\n"); return -1; }
  printf("\n");
  while (rg != NULL)
  {
      printf("rg[%ld->%ld]\n", rg->rg_start, rg->rg_end);
      rg = rg->rg_next;
  }
  printf("\n");
  return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
  struct vm_area_struct *vma = ivma;
 
  printf("print_list_vma: ");
  if (vma == NULL) { printf("NULL list\n"); return -1; }
  printf("\n");
  while (vma != NULL)
  {
      printf("vma[%ld->%ld]\n", vma->vm_start, vma->vm_end);
      vma = vma->vm_next;
  }
  printf("\n");
  return 0;
}

int print_list_pgn(struct pgn_t *ip)
{
  printf("print_list_pgn: ");
  if (ip == NULL) { printf("NULL list\n"); return -1; }
  printf("\n");
  while (ip != NULL )
  {
      printf("va[%d]-\n",ip->pgn);
      ip = ip->pg_next;
  }
  printf("\n");
  return 0;
}

int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end)
{
  int pgn_start, pgn_end;
  int pgit;

  if(end == -1){
    pgn_start = 0;
    struct vm_rg_struct *rg_node = caller->mm->mmap;
    if(rg_node == NULL) return -1;
    pgn_end = PAGING_PGN(rg_node->rg_end); 
  } else {
    pgn_start = PAGING_PGN(start);
    pgn_end = PAGING_PGN(end);
  }

  for(pgit = pgn_start; pgit < pgn_end; pgit++)
  {
     printf("print_pgtbl: %d - %08x\n", pgit, caller->mm->pgd[pgit]);
  }

  return 0;
}

#endif //ndef MM64