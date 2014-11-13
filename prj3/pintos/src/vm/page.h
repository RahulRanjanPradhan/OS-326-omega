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
// Load page from file/mmap to memory.
bool load_file (struct spt_entry *spte);
// Load page from swap to memory.
bool load_swap (struct spt_entry *spte);
// Add a file to supplemental page hash table of the current thread.
bool add_file_to_pt (struct file *file, int32_t ofs, uint8_t *upage,
			     uint32_t read_bytes, uint32_t zero_bytes, bool writable);
// Add a mmap to supplemental page hash table 
//  and mmaps of the current thread.
bool add_mmap_to_pt(struct file *file, int32_t ofs, uint8_t *upage,
			    uint32_t read_bytes, uint32_t zero_bytes);
// Grow stack if needed.
bool grow_stack (void *vaddr);




#endif /* /vm/page.h */