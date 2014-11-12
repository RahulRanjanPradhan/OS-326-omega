#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "vm/frame.h"
#include <hash.h>

// Max stack size = 4.2 MB
#define MAX_STACK_SIZE (1 << 25)

enum MEM_TYPE
{
	FILE,
	MMAP,
	SWAP,
	ERROR
};

struct spt_entry		//Supplemental page table entry
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

struct mmap_file {
  struct spt_entry *spte;
  int mapid;
  struct list_elem elem;
};


void page_table_init (struct hash *spt);
void page_table_destroy (struct hash *spt);
struct spt_entry* get_spte (void *vaddr);
bool load_page (struct spt_entry *spte);
bool load_swap (struct spt_entry *spte);
bool load_file (struct spt_entry *spte);
bool add_file_to_pt (struct file *file, int32_t ofs, uint8_t *upage,
			     uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool add_mmap_to_pt(struct file *file, int32_t ofs, uint8_t *upage,
			    uint32_t read_bytes, uint32_t zero_bytes);
bool grow_stack (void *vaddr);




#endif /* /vm/page.h */