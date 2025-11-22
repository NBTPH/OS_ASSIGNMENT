/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory physical module mm/mm-memphy.c
 */

#include "mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * MEMPHY_mv_csr - move MEMPHY cursor
 * @mp: memphy struct
 * @offset: offset
 */
int MEMPHY_mv_csr(struct memphy_struct *mp, int offset)
{
   int numstep = 0;

   mp->cursor = 0;
   while (numstep < offset && numstep < mp->maxsz)
   {
      /* Traverse sequentially */
      mp->cursor = (mp->cursor + 1) % mp->maxsz;
      numstep++;
   }

   return 0;
}

/*
 * MEMPHY_seq_read - read MEMPHY device
 * @mp: memphy struct
 * @addr: address
 * @value: obtained value
 */
int MEMPHY_seq_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL)
      return -1;

   if (!mp->rdmflg)
      return -1; /* Not in read mode */

   if (addr >= mp->maxsz)
      return -1; /* Out of range */

   /* Initialize cursor if needed, though seq_read implies reading from cursor */
   /* For random access simulation via seq_read, we move cursor */
   mp->cursor = addr;
   *value = mp->storage[addr];
   
   return 0;
}

/*
 * MEMPHY_read - read MEMPHY device (Random Access)
 * @mp: memphy struct
 * @addr: address
 * @value: obtained value
 */
int MEMPHY_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL || mp->storage == NULL)
      return -1;

   if (addr < 0 || addr >= mp->maxsz)
      return -1; /* Address out of bounds */

   *value = mp->storage[addr];

   return 0;
}

/*
 * MEMPHY_write - write MEMPHY device (Random Access)
 * @mp: memphy struct
 * @addr: address
 * @data: data to write
 */
int MEMPHY_write(struct memphy_struct *mp, int addr, BYTE data)
{
   if (mp == NULL || mp->storage == NULL)
      return -1;

   if (addr < 0 || addr >= mp->maxsz)
      return -1; /* Address out of bounds */

   mp->storage[addr] = data;

   return 0;
}

/*
 * MEMPHY_format - format MEMPHY device
 * @mp: memphy struct
 * @pagesz: page size
 * @punit: paging unit
 */
int MEMPHY_format(struct memphy_struct *mp, int pagesz)
{
    /* This function initializes the free frame list of the physical memory */
    if (mp == NULL || mp->maxsz == 0) 
        return -1;

    int numfp = mp->maxsz / pagesz;
    struct framephy_struct *newfst, *iter_fst;
    int iter = 0;

    /* Initialize the list head */
    /* We create the first node */
    iter_fst = malloc(sizeof(struct framephy_struct));
    iter_fst->fpn = iter;
    iter_fst->owner = NULL; // No owner yet
    iter_fst->fp_next = NULL;
    
    mp->free_fp_list = iter_fst;

    /* Create the rest of the list */
    for (iter = 1; iter < numfp; iter++)
    {
       newfst = malloc(sizeof(struct framephy_struct));
       newfst->fpn = iter;
       newfst->owner = NULL;
       
       /* Link: Add to the end or head? 
          Adding to head is O(1) but adding to tail preserves order.
          The snippet implies a tail addition logic or simple linking.
          Let's insert at head for simplicity (LIFO) or maintain tail pointer.
          Here we insert after the current node to keep order 0, 1, 2... */
       
       newfst->fp_next = NULL;
       iter_fst->fp_next = newfst;
       iter_fst = newfst;
    }

    return 0;
}

int MEMPHY_get_freefp(struct memphy_struct *mp, int *retfpn)
{
   struct framephy_struct *fp = mp->free_fp_list;

   if (fp == NULL)
      return -1; // No free frames available

   *retfpn = fp->fpn;
   mp->free_fp_list = fp->fp_next;

   /* MEMPHY is iteratively used up until its exhausted
    * No garbage collector acting then it not been released
    */
   free(fp);

   return 0;
}

int MEMPHY_dump(struct memphy_struct *mp)
{
   /* Dump memphy content mp->storage
    * for tracing the memory content
    */
    if (mp == NULL || mp->storage == NULL) return -1;

    printf("MEMPHY_dump:\n");
    for (int i = 0; i < mp->maxsz; i++) {
        if (mp->storage[i] != 0) {
            printf("  [%04d]: %02x\n", i, mp->storage[i]);
        }
    }
   return 0;
}

int MEMPHY_put_freefp(struct memphy_struct *mp, int fpn)
{
   struct framephy_struct *fp = mp->free_fp_list;
   struct framephy_struct *newnode = malloc(sizeof(struct framephy_struct));

   /* Create new node with value fpn */
   newnode->fpn = fpn;
   newnode->owner = NULL;
   
   /* Insert back to the head of the free list */
   newnode->fp_next = fp;
   mp->free_fp_list = newnode;

   return 0;
}

/*
 * init_memphy - initialize MEMPHY struct
 * @mp: memphy struct
 * @max_size: max size
 * @randomflg: random flag
 */
int init_memphy(struct memphy_struct *mp, int max_size, int randomflg)
{
   mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
   if (mp->storage == NULL) return -1;
   
   mp->maxsz = max_size;

   /* Initialize memory content to 0 */
   memset(mp->storage, 0, max_size * sizeof(BYTE));

   /* Initialize the format (free frame list) */
   /* PAGING_PAGESZ is assumed to be defined in mm.h */
   MEMPHY_format(mp, PAGING_PAGESZ);

   mp->rdmflg = (randomflg != 0)? 1 : 0;

   if (!mp->rdmflg) /* Not Ramdom access device, then it serial device*/
      mp->cursor = 0;

   return 0;
}

// #endif