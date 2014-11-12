#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"
#include "devices/block.h"
#include "threads/vaddr.h"
#include <bitmap.h>


// PGSIZE = 4KB; BLOCK_SECTOR_SIZE = 512B
#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

struct lock swap_lock;

struct block *swap_block;

struct bitmap *swap_bitmap;

void swap_init (void);
size_t swap_out (void *frame);
void swap_in (size_t used_index, void* frame);



#endif /* /vm/page.h */