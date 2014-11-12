#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"
#include "vm/page.h"



struct frame_entry
{
	void *frame;
	struct spte *spte;               /* Supplemental page table entry*/
	struct hash_elem hash_elem;
	struct thread *thread;
};

void frame_table_init (void);
void* frame_alloc (enum palloc_flags flags,struct spte *spte);
void frame_free (void *frame);
void frame_add_to_table (void *frame, struct spte *spte);
void* frame_evict (enum palloc_flags flags);