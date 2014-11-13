#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"
#include "vm/page.h"



struct frame_entry
{
	void *frame;
	struct spt_entry *spte;               /* Supplemental page table entry*/
	struct hash_elem hash_elem;
	struct thread *thread;
};

void frame_table_init (void);
void* frame_alloc (enum palloc_flags flags,struct spt_entry *spte);
void frame_free (void *frame);
void frame_add_to_ft (void *frame, struct spt_entry *spte);
void* frame_evict (enum palloc_flags flags);

#endif /* /vm/frame.h */