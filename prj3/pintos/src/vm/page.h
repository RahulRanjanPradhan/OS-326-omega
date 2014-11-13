#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "vm/frame.h"
#include <hash.h>

// Max stack size = 4.2 MB
#define MAX_STACK_SIZE (1 << 25)

// Page type
enum MEM_TYPE
{
	FILE,        // Read from a file(executable)
	MMAP,        // Read from a mmap
	SWAP,        // Stack or has been swap_out before
	HASH_FAIL    // Hash fail when insert a new hash_elem to hash table
};

//Supplemental page table entry
struct spt_entry		
{
	enum MEM_TYPE type;
	bool in_memory;
	bool writable;
	void *vaddr;
  bool locked;

  // Field for file
  struct file *file;
  size_t offset;
  size_t read_bytes;
  size_t zero_bytes;
  
  // Field for swap
  size_t swap_index;
	
  struct hash_elem elem;
};

// Mmap struct
struct mmap_file {
  struct spt_entry *spte;
  int mapid;
  struct list_elem elem;
};

// Initialize supplemental page hash table.
void page_table_init (struct hash *spt);
// Destroy supplemental page hash table
void page_table_destroy (struct hash *spt);
// From vaddr get spte in this thread
struct spt_entry* get_spte (void *vaddr);
// Load page from file/mmap/swap to memory.
bool load_page (struct spt_entry *spte);



#endif /* /vm/page.h */