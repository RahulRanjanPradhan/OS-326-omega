#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"
#include "vm/page.h"


/* Struct for frame entry. */
struct frame_entry
{
	void *frame;
	struct spt_entry *spte;               /* Supplemental page table entry*/
	struct hash_elem hash_elem;
	struct thread *thread;
};

/* Initialize frame table to compute hash values using frame_hash and
   compare hash elements using frame_less. */
void frame_table_init (void);
/* Allocate a frame for a page, if frame table is full, evict one.
 * Otherwise, add page to the frame table*/
void* frame_alloc (enum palloc_flags flags,struct spt_entry *spte);
/* Finds, removes, and returns an element, which frame is equal to frame
 * in frame table. Returns a null pointer if no equal element existed in the 
 * table. Finally free the frame_entry.
 */
void frame_free (void *frame);
/* Add a pointer which points to sub_page_table to frame_table*/
void frame_add_to_ft (void *frame, struct spt_entry *spte);
/* Use second chance algorithm to do the page replacement,
 * If swap is full, panic the kernel.
 */
void* frame_evict (enum palloc_flags flags);

#endif /* /vm/frame.h */