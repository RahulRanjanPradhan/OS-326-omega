#include <string.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"

// Hash func for supplemental page hash table.
static unsigned page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct spt_entry *spte = hash_entry(e, struct spt_entry, elem);
  return hash_int((int) spte->vaddr);
}

// Comparator func for supplemental page hash table.
static bool page_less_func (const struct hash_elem *a,
                            const struct hash_elem *b,
                            void *aux UNUSED)
{
  struct spt_entry *sa = hash_entry(a, struct spt_entry, elem);
  struct spt_entry *sb = hash_entry(b, struct spt_entry, elem);
  if (sa->vaddr < sb->vaddr)
  {
    return true;
  }
  return false;
}

// Func for hash destroy.
static void page_destroy_action (struct hash_elem *e, void *aux UNUSED)
{
  struct spt_entry *spte = hash_entry(e, struct spt_entry,
                                      elem);
  if (spte->in_memory)
  {
    frame_free(pagedir_get_page(thread_current()->pagedir, spte->vaddr));
    pagedir_clear_page(thread_current()->pagedir, spte->vaddr);
  }
  free(spte);
}

// Initialize supplemental page hash table.
void page_table_init (struct hash *spt)
{
  hash_init (spt, page_hash_func, page_less_func, NULL);
}

// Destroy supplemental page hash table
void page_table_destroy (struct hash *spt)
{
  hash_destroy (spt, page_destroy_action);
}

// From vaddr get spte in this thread
struct spt_entry *get_spte (void *vaddr)
{
  struct spt_entry spte;
  spte.vaddr = pg_round_down(vaddr);

  struct hash_elem *e = hash_find(&thread_current()->spt, &spte.elem);
  if (!e)
  {
    return NULL;
  }
  return hash_entry (e, struct spt_entry, elem);
}

// Load page from file/mmap/swap to memory.
bool load_page (struct spt_entry *spte)
{
  spte->locked = true;
  if (spte->in_memory)
  {
    return false;
  }
  switch (spte->type)
  {
  case FILE:
  case MMAP:
  {
    enum palloc_flags flags = PAL_USER;
    if (spte->read_bytes == 0)
    {
      flags |= PAL_ZERO;
    }
    uint8_t *frame = frame_alloc(flags, spte);
    if (!frame)
    {
      return false;
    }
    if (spte->read_bytes > 0)
    {
      lock_acquire(&fs_lock);
      if ((int) spte->read_bytes != file_read_at(spte->file, frame,
          spte->read_bytes,
          spte->offset))
      {
        lock_release(&fs_lock);
        frame_free(frame);
        return false;
      }
      lock_release(&fs_lock);
      memset(frame + spte->read_bytes, 0, spte->zero_bytes);
    }

    if (!install_page(spte->vaddr, frame, spte->writable))
    {
      frame_free(frame);
      return false;
    }

    spte->in_memory = true;
    return true;
    break;
  }

  case SWAP:
  {
    uint8_t *frame = frame_alloc (PAL_USER, spte);
    if (!frame)
    {
      return false;
    }
    if (!install_page(spte->vaddr, frame, spte->writable))
    {
      frame_free(frame);
      return false;
    }
    swap_in(spte->swap_index, spte->vaddr);
    spte->in_memory = true;
    return true;
  }
  default:
    break;
  }
  return false;
}

